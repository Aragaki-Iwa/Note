#include "CplexModel.h"

void dutyModel::solve() {
	try {
		_cplex.solve();		
		std::cout << "-----------cur model solved-----------\n";
		std::cout << "-----OBJ VALUE:" << _cplex.getObjValue() << "-----\n";

		/*std::cout << "lower num cover var = " << _cplex.getValue(_lowerNumCoverVar) << "\n";
		std::cout << "upper num cover var = " << _cplex.getValue(_upperNumCoverVar) << "\n";*/

	}
	catch (IloException& e) {
		std::cerr << "Concert exception caught:" << e << std::endl;
	}
	catch (...) {
		std::cerr << "Unknown exception caught" << std::endl;
	}

}
void dutyModel::checkCrewStatus(std::vector<CREW*>& crewSet, const CrewRules& rules) {		
	int size = crewSet.size();
	CREW* temp;	
	for (int i = 0; i < size; i++) {
		temp = crewSet[i];
		if (temp->workStatus->accumuCreditMin >= rules.maxCreditMin + rules.allowOverCreditMin) {
			temp->workStatus->dayoffMin = rules.minDayOffMin;
			crewSet[i] = crewSet[size - 1];
			crewSet[size - 1] = temp; //put crew who need day off to end of the crewSet.
		}
	}
}
void dutyModel::setCurSegSet(std::vector<Path*>& pathSet) {
	_cur_segSet.clear();
	_cur_segSet.shrink_to_fit();
	for (const auto& path : pathSet) {
		for (const auto& node : path->route) {
			if (node->nodeType == NodeType::seg 
				&& std::find(_cur_segSet.begin(), _cur_segSet.end(), node) == _cur_segSet.end()) {
				_cur_segSet.emplace_back(node);
			}
		}
	}
}
//NOTE:std::vector<Node*>& segSet应该只包括std::vector<Path*>& pathSet所包含的node
void dutyModel::init(/*const std::vector<Node*>& segSet, */std::vector<string>& specialAirport, 
	std::vector<Path*>& pathSet/*, std::vector<CREW*>& crewSet*/) {
	
	Path* path;
	string depArp, arvArp;
	for (int d = 0; d < pathSet.size(); d++) {
		path = pathSet[d];		
		/*_numSpecialFltOfDuty[i] = 0;
		for (const auto& node : path->route) {
			if (node->nodeType == NodeType::seg) {
				depArp = node->segment->getDepStation();
				arvArp = node->segment->getArrStation();
				if (std::find(specialAirport.begin(), specialAirport.end(), depArp) != specialAirport.end()
					|| std::find(specialAirport.begin(), specialAirport.end(), arvArp) != specialAirport.end()) {
					_numSpecialFltOfDuty[i] += 1;
				}				
			}
		}*/
		int size = _cur_segSet.size();
		//path->coverArray = new int[size]();//"()"使每个元素为0
		path->coverVec.resize(size);
		for (int i = 0; i < size; i++) {			
			if (std::find(path->route.begin(), path->route.end(), _cur_segSet[i]) != path->route.end()) {
				path->coverVec[i] = 1;
			} //不算dhd
			//for (const auto& node : path->route) {
			//	if (node->segment->getDBId() == _cur_segSet[i]->segment->getDBId()) {
			//		//path->coverArray[i] = 1;
			//		path->coverVec[i] = 1;
			//		break;
			//	}
			//}//算dhd
		}
	}
}
void dutyModel::reset(int iter) {		
	_model.end();
	_cplex.end();

	_dutyVarArr.end();
	_dhdVarArr.end();
	/*_fltCancelVarArr.end();
	_crewDutyVarMatrix.end();
	_crewFreeVarArr.end();*/

	_obj.end();
	_constraints.end();

	string model_name = "dutyModel(" + to_string(iter) + ")";
	_model = IloModel(_env, model_name.data());
	_dutyVarArr = IloNumVarArray(_env);
	_dhdVarArr = IloNumVarArray(_env);
	/*_fltCancelVarArr = IloNumVarArray(_env);
	_crewFreeVarArr = IloNumVarArray(_env);*/

	//_crewDutyVarMatrix = IloArray<IloNumVarArray>(_env);
	_obj = IloObjective(_env);
	_constraints = IloRangeArray(_env);	
	//用上面的，直接end比下面的clear快

	/*_model.remove(_obj);
	_model.remove(_constraints);
	_obj.end();
	
	_dutyVarArr.clear();
	_constraints.clear();*/
}

void dutyModel::buildModel(const Network& net, std::vector<Path*>& pathSet, std::vector<CREW*>& crewSet,
							std::vector<string>& specialAirport,
							const CrewRules& rules) {
	//std::vector<Node*> segSet(net.nodeSet->begin() + 2 + 2 * net.baseSet->size(), net.nodeSet->end());
	setCurSegSet(pathSet);
	init(/*segSet,*/ specialAirport, pathSet);

	//reset();

	int num_duty = pathSet.size();
	int num_seg = _cur_segSet.size();
	int num_crew = crewSet.size();

	/*---------------------------------add vars---------------------------------*/
	_dutyVarArr.add(IloNumVarArray(_env, num_duty, 0.0, 1.0, IloNumVar::Type::Int));
	_dhdVarArr.add(IloNumVarArray(_env, num_seg, 0.0, num_duty, IloNumVar::Type::Int));
	/*_fltCancelVarArr.add(IloNumVarArray(_env, num_seg, 0.0, 1.0, IloNumVar::Type::Int));
	_crewFreeVarArr.add(IloNumVarArray(_env, num_crew, 0.0, 1.0, IloNumVar::Type::Int));*/

	
	string varName("");
	//for (int d = 0; d < num_duty; d++) {
	//	varName = "[cur_duty:" + to_string(d) + "]";
	//	_dutyVarArr[d].setName(varName.data());
	//}
	//for (int f = 0; f < num_seg; f++) {
	//	varName = "[cur_seg_cancel:" + _cur_segSet[f]->segment->getFlightNumber() + "]";
	//	_fltCancelVarArr[f].setName(varName.data());
	//}
	//for (int c = 0; c < num_seg; c++) {
	//	varName = "[cur_crew_free:" + crewSet[c]->idCrew + "]";
	//	_crewFreeVarArr[c].setName(varName.data());
	//}
	////special skills
	//string specialSkill = "specialAirport";
	//std::vector<int> non_specialCrewSet;
	//for (int i = 0; i < num_crew; i++) {
	//	if (std::find((*crewSet[i]->rankAry)[0]->SkillSet.begin(), (*crewSet[i]->rankAry)[0]->SkillSet.end(), specialSkill)
	//		== (*crewSet[i]->rankAry)[0]->SkillSet.end()) { //特殊资质
	//		non_specialCrewSet.push_back(i);
	//	}		
	//}	
	//std::vector<int> non_specialDuty;
	//for (const auto& kv : _numSpecialFltOfDuty) {
	//	if (kv.second <= 0) { //this duty include special seg
	//		non_specialDuty.push_back(kv.first);
	//	}
	//}

	//int num_crew_duty_vars = 0;	
	//num_crew_duty_vars = num_crew;
	//_crewDutyVarMatrix.setSize(num_crew_duty_vars);
	//1.变量全都一一匹配，但是不具备资质的crew对应特殊duty的变量设其值为0
	/*2.TODO:还需要检查crew的累计工作时长，以确定是否需要day off。
	**2.1分配dutySet中最短时长的duty都违规，需要day off的crew不参与本次决策，直接更新状态
	**2.2否则，对crew仅配对其所能担当的duty，不能担当的则_crewDutyVarMatrix[c][d].setBounds(0.0, 0.0);
	*/
	////match special var	
	//for (int c = 0; c < num_crew; c++) {
	//	_crewDutyVarMatrix[c] = IloNumVarArray(_env, num_duty, 0.0, 1.0, IloNumVar::Type::Int);
	//			
	//	for (int d = 0; d < num_duty; d++) {
	//		varName = "[crew:" + crewSet[c]->idCrew + "], [duty:";
	//		varName += to_string(d) + "]";
	//		_crewDutyVarMatrix[c][d].setName(varName.data());
	//	}
	//}
	////filter by skills
	//for (const auto& c : non_specialCrewSet) {
	//	for (const auto& d : non_specialDuty) {
	//		_crewDutyVarMatrix[c][d].setBounds(0.0, 0.0);
	//	}
	//}
	////filter by credit minutes
	//for (int c = 0; c < num_crew; c++) {
	//	//若需要day off，则该crew直接day off，所以所有旗下的duty变量均为0
	//	if (crewSet[c]->workStatus->dayoffMin >= rules.minDayOffMin) {
	//		_crewDutyVarMatrix[c] = IloNumVarArray(_env, num_duty, 0.0, 0.0, IloNumVar::Type::Int);
	//	}
	//	//若暂未达到day off的时长
	//	else {
	//		for (int d = 0; d < num_duty; d++) {
	//			//但是若分配某些duty后需要day off。允许多工作一定的时间。
	//			//这里可以先对duty按时长降序排序，一旦if(... == false），则break。
	//			if (crewSet[c]->workStatus->accumuCreditMin + pathSet[d]->workMin 
	//				>= rules.maxCreditMin + rules.allowOverCreditMin) {
	//				_crewDutyVarMatrix[c][d].setBounds(0.0, 0.0);
	//			}
	//		}
	//	}
	//	
	//}
	
	/*_model.add(_dutyVarArr);
	_model.add(_fltCancelVarArr);
	_model.add(_crewDutyVarMatrix);*/
	
	/*---------------------------------add objs---------------------------------*/	
	//IloExpr obj_balance(_env);
	//IloExpr obj_special_gather(_env);
	//IloExpr obj_seg_cancel(_env);
	//IloExpr obj_crew_free(_env);
	IloExpr obj_sum_duty(_env);
	IloExpr obj_sum_dhd(_env);
	//IloExpr average_flymin(_env);
	//int sum_crew_flymin = 0;
	//for (int i = 0; i < num_crew; i++) {
	//	sum_crew_flymin += crewSet[i]->workStatus->accumuFlyMin;
	//}	
	for (int i = 0; i < num_duty; i++) {
		/*average_flymin += (pathSet[i]->flyMin) * _dutyVarArr[i];
		obj_special_gather += _numSpecialFltOfDuty[i] * _dutyVarArr[i];*/
		
		obj_sum_duty += _dutyVarArr[i];
	}
	//average_flymin += sum_crew_flymin;
	//average_flymin /= num_crew;


	/*for (int c = 0; c < num_crew; c++) {
		IloExpr crew_add_min(_env);
		for (int d = 0; d < num_duty; d++) {
			crew_add_min += pathSet[d]->flyMin * _crewDutyVarMatrix[c][d];
		}
		crew_add_min += crewSet[c]->workStatus->accumuFlyMin;
		obj_balance += (crew_add_min - average_flymin)*(crew_add_min - average_flymin);
		crew_add_min.end();
	}*/

	for (int f = 0; f < num_seg; f++) {
		//obj_seg_cancel += _fltCancelVarArr[f];
		obj_sum_dhd += _dhdVarArr[f];
	}
	//*for (int i = 0; i < num_crew; i++) {
	//	obj_crew_free += _crewFreeVarArr[i];
	//}
	//obj_balance /= (num_crew * num_crew);
	//obj_special_gather *= _specialSegGatherWight;*/
	//obj_seg_cancel *= _penaltyFltCancel;
	////obj_crew_free *= _penaltyCrewFree;
	//obj_sum_duty *= _costDuty;

	/*std::cout << "obj_balance:" << obj_balance << "\n";
	std::cout << "obj_special_gather:" << obj_special_gather << "\n";
	std::cout << "obj_seg_cancel:" << obj_seg_cancel << "\n";*/
	/*std::cout << "obj_crew_free:" << obj_crew_free << "\n";
	std::cout << "obj_sum_duty:" << obj_sum_duty << "\n";*/

	//_obj = IloMinimize(_env, _upperNumCoverVar- _lowerNumCoverVar, "<let cover num close to 3>");
	_obj = IloMinimize(_env, obj_sum_duty + obj_sum_dhd, "<min num of duty, penalty dhd>");
	_model.add(_obj);
	//_model.add(IloMinimize(_env, /*obj_balance - obj_special_gather + obj_seg_cancel +*/ obj_crew_free + obj_sum_duty
	//	, "<flymin balance and gather special segment>"));
	obj_sum_duty.end();
	obj_sum_dhd.end();

	/*average_flymin.end();
	obj_balance.end();
	obj_special_gather.end();
	obj_seg_cancel.end();
	obj_crew_free.end();*/

	/*---------------------------------constraints---------------------------------*/
	string ctName("");
	////1.each crew(CAP and FO) must work on a duty
	//map<int, CREW*> id_CAP;
	//map<int, CREW*> id_FO;
	//for (int c = 0; c < num_crew; c++) {
	//	if (crewSet[c]->division == "P") {
	//		if (crewSet[c]->rankAry->front()->rank == "CAP") {
	//			id_CAP[c] = crewSet[c];
	//		}
	//		else if (crewSet[c]->rankAry->front()->rank == "FO") {
	//			id_FO[c] = crewSet[c];
	//		}
	//		else {
	//			int o = 0;
	//		}

	//		IloExpr ct1(_env);
	//		ctName = "";
	//		for (int d = 0; d < num_duty; d++) {
	//			ct1 += _crewDutyVarMatrix[c][d];
	//		}
	//		ct1 += _crewFreeVarArr[c];
	//		
	//		ctName += "(x=1): crew<" + crewSet[c]->idCrew + ">";
	//		ct1.setName(ctName.data());
	//		
	//		_constraints.add(ct1 == 1);
	//		ct1.end();
	//	}
	//}
	////one duty, only one FO and one CAP
	//for (int d = 0; d < num_duty; d++) {
	//	IloExpr ct_cap(_env);
	//	for (auto& id_cap_pair : id_CAP) {
	//		ct_cap += _crewDutyVarMatrix[id_cap_pair.first][d];
	//	}
	//	_constraints.add(ct_cap <= 1);
	//	ct_cap.end();
	//}
	//for (int d = 0; d < num_duty; d++) {
	//	IloExpr ct_fo(_env);
	//	for (auto& id_fo_pair : id_FO) {
	//		ct_fo += _crewDutyVarMatrix[id_fo_pair.first][d];
	//	}
	//	_constraints.add(ct_fo <= 1);
	//	ct_fo.end();
	//}

	//4.rank match constraints:
	//only particular ranks can work together
	//namely some rank can't work togrther
	//x[c1][d] + x[c2][d] <= 1
	//CREW_RANK *cap_rank, *fo_rank;
	//for (auto& id_cap : id_CAP) {
	//	cap_rank = crewSet[id_cap.first]->rankAry->front();
	//	for (auto& id_fo : id_FO) {
	//		fo_rank = crewSet[id_fo.first]->rankAry->front();
	//		if (checkRankMatch(cap_rank, fo_rank, rules.rankCombinationSet)) {
	//			for (int d = 0; d < num_duty; d++) {
	//				IloExpr ct_mutual(_env);
	//				ct_mutual += (_crewDutyVarMatrix[id_cap.first][d] + _crewDutyVarMatrix[id_fo.first][d]);
	//				//_constraints.add(ct_mutual <= 1);
	//				ct_mutual.end();
	//			}				
	//		}
	//	}
	//}
	//2.x[c][d] <= y[d]:a duty can be assigned to a crew which choosed in opt before
	//for (int c = 0; c < num_crew; c++) {
	//	for (int d = 0; d < num_duty; d++) {
	//		ctName = "";
	//		IloExpr ct2(_env);
	//		ct2 += (_crewDutyVarMatrix[c][d] - _dutyVarArr[d]);
	//		ctName += "(x<=y): crew <" + crewSet[c]->idCrew + ">,duty<" + to_string(d) + ">";
	//		ct2.setName(ctName.data());
	//		//_constraints.add(ct2 <= 0);
	//		ct2.end();
	//	}
	//}	
	//3.set cover constraints
	for (int f = 0; f < num_seg; f++) {
		ctName = "";
		IloExpr ct3(_env);
		for (int d = 0; d < num_duty; d++) {
			//ct3 += pathSet[d]->coverArray[f] * _dutyVarArr[d];
			ct3 += pathSet[d]->coverVec[f] * _dutyVarArr[d];
		}
		//ct3 += _fltCancelVarArr[f];
		ctName += "(f=1):<" + _cur_segSet[f]->segment->getFlightNumber() + ">";
		ct3.setName(ctName.data());
		/*_constraints.add(ct3 - _lowerNumCoverVar >= 0);
		_constraints.add(ct3 - _upperNumCoverVar <= 0);*/
		_constraints.add(ct3 - _dhdVarArr[f] == 1);
		ct3.end();
	}		
	_model.add(_constraints);
	
	_cplex = IloCplex(_model);
	/*string model_name(_model.getName());
	model_name += ".lp";
	_cplex.exportModel(model_name.data());*/
}

bool dutyModel::checkRankMatch(CREW_RANK* cap, CREW_RANK* fo, const std::unordered_set<string>& rankCombinationSet) {
	std::string combination(cap->position);
	combination += "-" + fo->position;
	return rankCombinationSet.find(combination) == rankCombinationSet.end();
}

void dutyModel::post_process(/*Network& net, */std::vector<Path*>& pathSet, std::vector<CREW*>& crewSet) {
	//1.标记node
	//2.存储当前决策duty
	_decided_dutySet.clear();
	int dutyVarValue;
	for (int d = 0; d < pathSet.size(); d++) {
		dutyVarValue = _cplex.getValue(_dutyVarArr[d]);
		if (dutyVarValue > 0.5) {			
			_decided_dutySet.emplace_back(pathSet[d]);
		}
	}
	////2.实际指派duty给crew
	//std::cout << "this iter, assigned duty to crews: \n";
	//int crew_duty_varValue;
	//for (int c = 0; c < crewSet.size(); c++) {
	//	for (int d = 0; d < pathSet.size(); d++) {
	//		crew_duty_varValue = _cplex.getValue(_crewDutyVarMatrix[c][d]);
	//		if (crew_duty_varValue > 0.5) {
	//			pathSet[d]->work_crewSet.emplace_back(crewSet[c]);
	//			std::cout << "crew < " << crewSet[c]->idCrew << " > to duty < " << to_string(d) << " >\n";
	//		}
	//	}
	//}
	//updateCrewStatus();
}
void dutyModel::updateCrewStatus() {
	for (auto& duty : _decided_dutySet) {
		for (auto& crew : duty->work_crewSet) {
			crew->workStatus->accumuCreditMin += duty->workMin;
			crew->workStatus->accumuFlyMin += duty->flyMin;
			crew->workStatus->restStation = duty->route.back()->arvStation;
		}
	}
}

std::vector<Path*>* dutyModel::getDecidedDutySet() {
	return &_decided_dutySet;
}
std::vector<Node*>* dutyModel::getCurSegNodeSet() {
	return &_cur_segSet;
}