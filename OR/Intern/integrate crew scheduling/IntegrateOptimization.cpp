#include "IntegrateOptimization.h"

void RollingOpt::optimize() {
	
	_net->createNetwork(_segSet, _baseSet, *_rules);
	std::cout << "----------create Network finished----------\n";

	initialStartNodeSet();
	_assigner.init(&_crewSet, _rules, &_curCAPCrewSet, &_curFOCrewSet, &_curAttendantCrewSet);

	int num_iter = 1;
	try {
		while (!termination() && num_iter <= 11) {
			std::cout << "----------" << num_iter << "th iter started----------\n";
			/*enumerate duty*/
			_pathFinder.findPathSet(_net, _cur_start_nodeSet, *_rules);
			_cur_dutySet = *_pathFinder.getPathSet();//此处注意pathFinder中pathSet的内存释放
			
			//删除掉只有一个segment，且为dhd的duty
			removeDhdDuty(_cur_dutySet);

			/*set covering: get decidedDutySet*/
			_dutyModel.buildModel(*_net, _cur_dutySet, _crewSet, _special_airportSet, *_rules);
			_dutyModel.solve();
			_dutyModel.post_process(_cur_dutySet, _crewSet);					
			_dutyModel.reset(num_iter); 
			
			/*assignment*/
			_cur_dutySet = *_dutyModel.getDecidedDutySet();
			
		
			std::vector<Node*>* curSegSet = _dutyModel.getCurSegNodeSet();

			std::vector<Node*> duty_coveredNodes;
			for (int d = 0; d < _cur_dutySet.size(); d++) {
				for (const auto& node : _cur_dutySet[d]->route) {
					if (node->nodeType == NodeType::seg
						&& std::find(duty_coveredNodes.begin(), duty_coveredNodes.end(), node) == duty_coveredNodes.end()) {
						duty_coveredNodes.emplace_back(node);
					}
				}
			}
			std::cout << "set covering error\n";
			for (const auto& node1 : *curSegSet) {
				if (std::find(duty_coveredNodes.begin(), duty_coveredNodes.end(), node1) == duty_coveredNodes.end()) {
					std::cout << "id = " << node1->segment->getDBId() << " flt = " << node1->segment->getFlightNumber() << "\n";
				}
			}




			_assigner.receiveInput(&_cur_dutySet, curSegSet);
			_assigner.labelDecidedDuty(_special_airportSet);
			
			_assigner.solve();
			_cur_dutySet = *_assigner.getAssignedDutySet();

			std::cout << "----------" << num_iter++ << "th iter finished----------\n\n";
			_decided_dutySet.insert(_decided_dutySet.end(), _cur_dutySet.begin(), _cur_dutySet.end());
			updateStartNodeSet(num_iter);
		}
	}
	catch (IloException& e) {
		std::cout << "iloException: " << e.getMessage() << "\n";
	}
	catch (...) {
		std::cerr << "<<<<<<<<<<<<<<<<<<<< OPTIMIZE FAILED >>>>>>>>>>>>>>>>>>>>" << std::endl;
		std::cerr << "---------------Unknown exception caught---------------" << std::endl;
	}
	
}

void RollingOpt::inputData(std::map<string, std::vector<void*>>& dataSet, const std::vector<string>& objNameSet) {
	for (const auto& obj_name : objNameSet) {
		if (obj_name == "Flight") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_segSet); }
		else if (obj_name == "Base") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_baseSet); }
		else if (obj_name == "Crew") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crewSet); }
		else if (obj_name == "CrewRank") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crew_rankSet); }
		else if (obj_name == "CrewBase") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crew_baseSet); }
		else { std::cerr << "ERROR: invalid obj data name" << obj_name << std::endl; }
	}
}
void RollingOpt::setRules(CrewRules& rules) {
	_rules = &rules;
	rules.rankCombinationSet = _rankCombinationSet;
}
void RollingOpt::init() {
	/*_crewSet = *_inputHandler.getPilotSet(_crewSet);
	_special_airportSet = _inputHandler.*/	
	_inputHandler.matchCrewAndRank(&_crewSet, &_crew_rankSet);
	_inputHandler.matchCrewAndBase(&_crewSet, &_crew_baseSet);
	for (auto& crew : _crewSet) {
		_inputHandler.sortCrewRank(crew->rankAry);
	}

	_crewSet = *_inputHandler.getPilotSet(_crewSet);

	clusterCrewMeb();
}
void RollingOpt::clusterCrewMeb() {
	string crew_division;
	CREW* crew;
	for (int c = 0; c < _crewSet.size(); c++) {
		crew = _crewSet[c];
		crew_division = crew->division;
		if (crew_division == "P") {
			if (crew->rankAry->front()->rank == "CAP") {
				_curCAPCrewSet.emplace_back(crew);
			}
			else if (crew->rankAry->front()->rank == "FO") {
				_curFOCrewSet.emplace_back(crew);
			}
		}
		else if (crew_division == "C") {
			_curAttendantCrewSet.emplace_back(crew);
		}
	}
}

void RollingOpt::randomSet_specialAirport() {
	_inputHandler.getAirportSet(_segSet);
	_special_airportSet = *_inputHandler.createSpecialArpSet();
	//debug
	std::cout << "-----------special airport-----------\n";
	for (const auto& arp : _special_airportSet) {
		std::cout << arp << "\n";
	}
}
void RollingOpt::randomSet_crewSkills(double percent) {
	string skill("specialAirport");	
	int num_special_crew = 0;
	int size = _crewSet.size();
	CREW_RANK* rank;
	std::cout << "-----------special airport-----------\n";
	for (auto& crew : _crewSet) {
		rank = (*crew->rankAry)[0];
		rank->SkillSet.clear();
		if (rand() % 2 == 1) {
			rank->SkillSet[skill] = 1;
			++num_special_crew;

			std::cout << "crew <" << crew->idCrew << "> able to fly: " << skill << "\n";
		}

		if (num_special_crew / size >= percent) {
			break;
		}
	}
}
void RollingOpt::set_rankCombination() {
	vector<string> CAP_positions{ "C1", "C4", "T1", "T2", "T3", "K1", "K2" };
	vector<string> FO_positions{ "E1", "F1", "F2", "F3", "F4", "F5", "F6", "J1", "J2" };	
	
	std::cout << "----------Rank Combinations----------\n";
	stringstream combi;
	for (int i = 0; i < 2; i++) {	
		for (int j = 1; j < FO_positions.size(); j++) {
			combi.str("");
			combi << CAP_positions[i] << "-" << FO_positions[j];
			std::cout << combi.str() << "\n";
			_rankCombinationSet.insert(combi.str());
		}
	}
	for (int i = 2; i < 5; i++) {
		combi.str("");
		combi << CAP_positions[i] << "-" << FO_positions[0];
		std::cout << combi.str() << "\n";
		_rankCombinationSet.insert(combi.str());
	}
	for (int i = 5; i < CAP_positions.size(); i++) {		
		for (int j = 0; j < FO_positions.size() - 2; j++) {
			combi.str("");
			combi << CAP_positions[i] << "-" << FO_positions[j];
			std::cout << combi.str() << "\n";
			_rankCombinationSet.insert(combi.str());
		}
	}

	//debug
	
	/*for (auto iter = _rankCombinationSet.begin(); iter != _rankCombinationSet.end(); iter++) {
		std::cout << *iter << "\n";
	}*/
}


void RollingOpt::removeDhdDuty(std::vector<Path*>& dutySet) {
	for (auto iter = dutySet.begin(); iter != dutySet.end();) {
		if ((*iter)->route.size() == 1 && (*iter)->route.back()->nodeType == NodeType::dhd) {
			iter = dutySet.erase(iter);
		}
		else {
			iter++;
		}
	}
}

void RollingOpt::initialStartNodeSet() {	
	//_cur_start_nodeSet.emplace_back(_net->nodeSet->front());
	//取第一天的点
	auto first_node = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size();
	time_t first_day = getStartTimeOfDay((*first_node)->startDtLoc);
	time_t node_inDay;

	for (auto iter = first_node; iter != _net->nodeSet->end(); iter++) {
		node_inDay = getStartTimeOfDay((*iter)->startDtLoc);
		if (node_inDay == first_day) {
			_cur_start_nodeSet.emplace_back((*iter));
		}	
	}
}

bool RollingOpt::termination() {
	//从后往前扫描net
	for (const auto& node : *_net->nodeSet) {
		if (!node->assigned) {
			return false;
		}
	}
	return true;
}
void RollingOpt::updateStartNodeSet(int iterDay) {	
	_cur_start_nodeSet.clear();
	//1.上一阶段duty结束点（segment）的后继节点
	//2.加上，在所有结束点中最晚的点之前出发的，未被assign的segment
	time_t last_arv = 0;
	Node* last;
	Node* end;
	for (const auto& duty : _cur_dutySet) {
		end = duty->route.back();
		for (auto& arc : *end->outArcSet) {
			if ((arc->endNode->nodeType == NodeType::seg || arc->endNode->nodeType == NodeType::dhd)
				&& std::find(_cur_start_nodeSet.begin(), _cur_start_nodeSet.end(), arc->endNode) == _cur_start_nodeSet.end()) {
				_cur_start_nodeSet.emplace_back(arc->endNode);
			}
		}

		if (end->endDtUtc > last_arv) {
			last_arv = end->endDtUtc;
			last = end;
		}
	}

	auto last_index = std::find(_net->nodeSet->begin(), _net->nodeSet->end(), last);
	auto seg_begin = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size();
	for (auto iter = last_index; iter != seg_begin; iter--) {
		if ((*iter)->nodeType == NodeType::seg && (*iter)->assigned == false && (*iter)->startDtUtc <= last_arv) {
			_cur_start_nodeSet.emplace_back(*iter);
		}
	}
	end = nullptr;
	//3.当前阶段对应天出发的点	
	time_t cur_startDay = _net->nodeSet->front()->startDtUtc + 3600 * 24 * (iterDay - 1);
	time_t node_inDay;
	for (auto iter = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size(); iter != _net->nodeSet->end(); iter++) {
		node_inDay = getStartTimeOfDay((*iter)->startDtLoc);
		if (node_inDay == cur_startDay && (*iter)->assigned == false
			&& std::find(_cur_start_nodeSet.begin(), _cur_start_nodeSet.end(), (*iter)) == _cur_start_nodeSet.end()) {
			_cur_start_nodeSet.emplace_back(*iter);
		}
		else if (node_inDay > cur_startDay) {
			break;//因为node是按排序来的
		}
	}
}