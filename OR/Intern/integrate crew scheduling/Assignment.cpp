#include "Assignment.h"

Solution::Solution(const int numRow, const int numColumn) {
	/*CrewDutyMatrix = new int*[numRow];
	for (size_t i = 0; i < numRow; i++) {
		CrewDutyMatrix[i] = new int[numColumn]();
	}*/
	CrewDutyMatrix.resize(numRow);
	for (size_t i = 0; i < numRow; i++) {
		CrewDutyMatrix[i].resize(numColumn);
	}	

	//adj
	CrewDutyAdj.clear();
	DutyCrewAdj.clear();
	CrewDutyAdj.resize(numRow);
	DutyCrewAdj.resize(numColumn);
}

int Assigner::solve() {	
	//initialization
	initFlyMints();
	clusterDutyByDay();

	//optimization
	//initialSolution();
	//initialSolution_basedCrew();
	initialSolution_baseCrewAndDay();
	
	calObjValue(*_initialSoln);


	_optSoln = new Solution(*_initialSoln);
	delete _initialSoln;
	postProcess();

	std::cout << "opt solution's uncovered flt: " << _optSoln->obj.fltCancelCost << "\n";

	std::cout << "from _decidedDutySet which inlude " << _decidedDutySet->size() << " num of dutys\n";
	std::cout << "got _assignedDutySet which inlude " << _assignedDutySet->size() << " num of dutys\n";

	return 1;
}
/*public, for init*/
void Assigner::init(std::vector<CREW*>* p_crewSet, CrewRules* p_rules, std::vector<std::vector<int>>* p_crewMutualMatrix,
	std::vector<CREW*>* p_curCAPCrewSet,
	std::vector<CREW*>* p_curFOCrewSet,
	std::vector<CREW*>* p_curAttendantCrewSet) {
	_crewSet = p_crewSet;
	_rules = p_rules;
	_curCAPCrewSet = p_curCAPCrewSet;
	_curFOCrewSet = p_curFOCrewSet;
	_curAttendantCrewSet = p_curAttendantCrewSet;

	_crewMutualMatrix = *p_crewMutualMatrix;
}
void Assigner::receiveSetCoverInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet) {	
	_decidedDutySet = p_decidedDutySet;	
	_curSegNodeSet = p_curSegNodeSet;	
}
void Assigner::labelDecidedDuty(std::vector<string>& specialAirport) {
	int duty_size = _decidedDutySet->size();
	Path* duty;
	//先找出特殊资质的duty,目前只有特殊机场要求
	string depArp, arvArp;
	for (int d = 0; d < duty_size; d++) {
		duty = (*_decidedDutySet)[d];
		duty->specialCredentials["SpecialAirport"] = 0;
		for (const auto& node : duty->route) {
			if (node->nodeType == NodeType::seg) {
				depArp = node->segment->getDepStation();
				arvArp = node->segment->getArrStation();
				if (std::find(specialAirport.begin(), specialAirport.end(), depArp) != specialAirport.end()
					|| std::find(specialAirport.begin(), specialAirport.end(), arvArp) != specialAirport.end()) {
					duty->specialCredentials["SpecialAirport"] += 1;					
				}
			}
		}
	}

	//按duty时长降序排序，执勤时
	std::sort(_decidedDutySet->begin(), _decidedDutySet->end()
		, [](const Path* a, const Path* b) {return a->workMin > b->workMin; });

}

/*private*/
//整个指派执行一次即可
void Assigner::clusterDutyByDay() {	
	//对_decidedDutySet按执勤时间降序排序
	//这样之后按天分duty也就已经排好序了
	std::sort(_decidedDutySet->begin(), _decidedDutySet->end()
		, [](const Path* a, const Path* b) {return a->workMin > b->workMin; });

	_decidedDutyInDaysID.clear();
	
	Path* duty;
	for (int d = 0; d < _decidedDutySet->size(); d++) {
		duty = (*_decidedDutySet)[d];
		_decidedDutyInDaysID[duty->startDate].emplace_back(d); //存放的是index
	}
		
}

void Assigner::initFlyMints() {
	_CrewFlyMints.clear();
	_CrewFlyMints.reserve(_crewSet->size());

	_initial_sumFlyMint = 0;

	for (const auto& crew : *_crewSet) {
		_initial_sumFlyMint += crew->workStatus->accumuFlyMin;
		_CrewFlyMints.emplace_back(crew->workStatus->accumuFlyMin);
	}
}


void Assigner::initCrewDutyColumn(std::vector<int>& decidedDutysID) {
	for (auto& crew : *_crewSet) {
		crew->workStatus->assigned = false;

		crew->workStatus->setDutyColumn(_decidedDutySet->size());
	}

	int crew_size = _crewSet->size();	
	int duty_size = decidedDutysID.size();

	//CREW* temp_crew;
	int gap_crew_duty = 0;//crew当前时间点和欲担当的duty开始时间的间隔
	bool exist_feasible_duty = false;
	for (/*int c = 0; c < crew_size; c++*/
		auto temp_crew = _crewSet->begin(); temp_crew != _crewSet->end(); temp_crew++) {
		//temp_crew = crewSet[c];
		 
		if ((*temp_crew)->workStatus->accumuCreditMin > _rules->maxCreditMin ||
			(*temp_crew)->workStatus->accumuFlyMin > _rules->maxWeekFlyMin) {
			//need a day off
			(*temp_crew)->workStatus->accumuCreditMin = 0;
			(*temp_crew)->workStatus->assigned = true; //assigned a day off
			(*temp_crew)->workStatus->endDtLoc += _rules->minDayOffMin;
			(*temp_crew)->workStatus->accumuFlyMin = 0;
		}
		else  { //执勤,飞行时间小于规定的最大
			Path* duty;
			int d = 0;
			for (int i = 0; i < duty_size; i++) {				
				d = decidedDutysID[i];
				duty = (*_decidedDutySet)[d];

				if ((*temp_crew)->workStatus->restStation != "" && (*temp_crew)->workStatus->restStation != duty->startStation) {
					//=="",说明是初次迭代，不需要满足空间接续。不过实际上crew的初始状态中是有一个计划周期开始时的所在地的信息，这里先不考虑
					continue;
				}
				/*recent added 7-18*/
				//可以执行到下面，说明空间接续满足				
				if ((*temp_crew)->idCrew == "001551" && (*temp_crew)->workStatus->CreditedDutySet.size() > 0) {
					int gg = 0;
				}

				gap_crew_duty = (duty->startDtLoc - (*temp_crew)->workStatus->endDtLoc) / 60;
				/*if (gap_crew_duty <= 0) {
					continue;
				}*/

				if (gap_crew_duty > 72 * 24 * 60) {
					break;
				}
				else if (0 < gap_crew_duty && gap_crew_duty < _rules->minDayOffMin) {
					//不需要day off，就是duty之间的正常接续
					//但预先判断一下若担当该duty，是否会超时
					if ((*temp_crew)->workStatus->accumuCreditMin + gap_crew_duty + duty->workMin > _rules->maxCreditMin
						|| (*temp_crew)->workStatus->accumuFlyMin + duty->flyMin > _rules->maxWeekFlyMin) {
						break;
					}
				}
				else if (_rules->minDayOffMin <= gap_crew_duty && gap_crew_duty <= 72 * 24 * 60) { 
					//day off min set in [36, 72],不允许太长
					//可担当该duty，但是与duty之间的间隔达到了day off的时长，说明进行了day off
					//(*temp_crew)->workStatus->accumuCreditMin = 0;
					//NOTE:day off的时间更新放在做了决策后再判断，7-18-23：52
					////(*temp_crew)->workStatus->dayoffMin = gap_crew_duty;
				}
				/*end recent added 7-18*/
								
				//if ((*temp_crew)->workStatus->accumuCreditMin + duty->workMin >= _rules->maxCreditMin
				//	|| (*temp_crew)->workStatus->accumuCreditMin + duty->workMin + _rules->allowOverCreditMin >= _rules->maxCreditMin) {
				//	break; //不能担当该duty，因为会超时.d越大，时长越长，所以不用再往下查找，直接break
				//}

				exist_feasible_duty = true;
				(*temp_crew)->workStatus->dutyColumn[d] = 1; //先不考虑资质，只考虑空间和时间，所有crew都可以担当任意duty

			}		
		}
		
		if (exist_feasible_duty == false) {
			//当前天找不到可担当的duty，不一定需要day off			
			(*temp_crew)->workStatus->assigned = true; //assigned a break
		}
		else {
			//考虑资质：special duty只有special crew可以担当
			Path* duty;
			int d = 0;
			bool exist_feasible_duty = false;
			for (int i = 0; i < duty_size; i++) {				
				d = decidedDutysID[i];
				duty = (*_decidedDutySet)[d];
				
				string special = "SpecialAirport";
				if (duty->specialCredentials[special] >= 1 &&
					(*temp_crew)->rankAry->front()->SkillSet[special] != 1) {
					(*temp_crew)->workStatus->dutyColumn[d] = 0;
				}
			}

		}			
	}
	//sort _curCrewSet，执勤时间升序排序
	/*std::sort(_crewSet->begin(), _crewSet->end()
		, [](const CREW* a, const CREW* b) {return a->workStatus->accumuCreditMin < b->workStatus->accumuCreditMin; });*/
	sortCrewSet();
}
void Assigner::setCrewDutyMatchAdj() {
	_dutyCrewAdj.clear();
	_crewDutyAdj.clear(); 
	_dutyCrewAdj.shrink_to_fit();
	_crewDutyAdj.shrink_to_fit();

	int crew_size = _crewSet->size();
	int duty_size = _decidedDutySet->size();
	_dutyCrewAdj.resize(duty_size);
	_crewDutyAdj.resize(crew_size);

	for (int c = 0; c < crew_size; c++) {
		for (int d = 0; d < duty_size; d++) {
			if ((*_crewSet)[c]->workStatus->dutyColumn[d] == 1) {
				_dutyCrewAdj[d].emplace_back(c);

				_crewDutyAdj[c].emplace_back(d);
			}
		}
	}
}

void Assigner::sortCrewSet() {
	/*交叉从CAP 、FO出发挑选duty*/
	_crewSet->clear();
	std::sort(_curCAPCrewSet->begin(), _curCAPCrewSet->end()
		, [](const CREW* a, const CREW* b) {return a->workStatus->accumuCreditMin < b->workStatus->accumuCreditMin; });
	std::sort(_curFOCrewSet->begin(), _curFOCrewSet->end()
		, [](const CREW* a, const CREW* b) {return a->workStatus->accumuCreditMin < b->workStatus->accumuCreditMin; });

	int i = 0, j = 0;	
	while (i < _curCAPCrewSet->size() && j < _curFOCrewSet->size()) {
		_crewSet->emplace_back((*_curCAPCrewSet)[i++]);
		_crewSet->emplace_back((*_curFOCrewSet)[j++]);
	}
	while (i < _curCAPCrewSet->size()) { _crewSet->emplace_back((*_curCAPCrewSet)[i++]); }
	while (j < _curFOCrewSet->size()) { _crewSet->emplace_back((*_curFOCrewSet)[j++]); }
}




/*----------algorithm-process----------*/

void Assigner::initialSolution() {
	_initialSoln = new Solution(_crewSet->size(), _decidedDutySet->size());
	_initialSoln->CrewDutyAdj.resize(_crewSet->size());
	_initialSoln->DutyCrewAdj.resize(_decidedDutySet->size());

	//debug
	Path* path;
	for (int i = 0;i < _decidedDutySet->size();i++) {
		path = (*_decidedDutySet)[i];
		for (const auto& node : path->route) {
			if (node->nodeType == NodeType::seg) {
				if (node->segment->getDBId() == 54508631
					|| node->segment->getDBId() == 54508237
					|| node->segment->getDBId() == 54503251) {
					int ddd = 0;
				}
			}
			
		}
	}


	int duty_size = _decidedDutySet->size();
	bool finished = true;
	bool duty_assigned = false;
	int c1 = 0, c2 = 0;
	size_t d = 0;
	do {
		int i = 0;
		int crew_size = _dutyCrewAdj[d].size();
		duty_assigned = false;
		while (i < crew_size && duty_assigned == false) {
			c1 = _dutyCrewAdj[d][i];
			if ((*_crewSet)[c1]->workStatus->assigned == true) {
				++i;
				continue;
			}

			for (int j = i + 1; j < crew_size; j++) {
				c2 = _dutyCrewAdj[d][j];
				//if not mutual and have not be assigned to some duty
				if (_crewMutualMatrix[c1][c2] == 1 					
					&& (*_crewSet)[c2]->workStatus->assigned == false) { 
					
					(*_crewSet)[c1]->workStatus->assigned = true;
					(*_crewSet)[c2]->workStatus->assigned = true;
					/*(*_decidedDutySet)[d]->work_crewSet.emplace_back((*_crewSet)[c1]);
					(*_decidedDutySet)[d]->work_crewSet.emplace_back((*_crewSet)[c2]);*/

					_initialSoln->CrewDutyMatrix[c1][d] = 1;
					_initialSoln->CrewDutyMatrix[c2][d] = 1;
					_initialSoln->CrewDutyAdj[c1].emplace_back(d);
					_initialSoln->CrewDutyAdj[c2].emplace_back(d);
					_initialSoln->DutyCrewAdj[d].emplace_back(c1);
					_initialSoln->DutyCrewAdj[d].emplace_back(c2);


					for (auto& node : (*_decidedDutySet)[d]->route) {
						if (node->nodeType == NodeType::seg) { 
							node->assigned = true; 
						}
					}

					duty_assigned = true;
					break;
				}
			}
			i++;
		}
		
		//check if all node in _curSegNodeSet were assigned		
		finished = true;
		finished = isCurSegCoverFinished();
		
		d++;
		if (d == 80 || d == 95 || d == 100) {
			int mmmm = 0;
		}

	} while (d < duty_size && finished == false);
	//if not feasible?
	if (finished == false) {
		//debug
		_uncoveredFltFile = to_string(duty_size) + _uncoveredFltFile;
		_outStream.open(_uncoveredFltFile);
		int dd = 0;
		for (const auto& node : *_curSegNodeSet) {
			if (node->assigned == false) {
				//std::cout << "id<" << node->segment->getDBId() << ">, fltNum<" << node->segment->getFlightNumber() << ">\n";
				_outStream << _fltParser.toCsv(_fltParser.getDefaultHeaders(), node->segment) << "\n";
			}
		}
		_outStream.close();
	}
}

int Assigner::initialSolution_basedCrew() {
	int crew_size = _crewSet->size();
	int duty_size = _decidedDutySet->size();
	_initialSoln = new Solution(crew_size, duty_size);
	_initialSoln->CrewDutyAdj.resize(crew_size);
	_initialSoln->DutyCrewAdj.resize(duty_size);

	bool finished = true;
	int c = 0;
	int d = 0;
	CREW* crew;
	Path* duty;
	
	do {		
		crew = (*_crewSet)[c];
		duty_size = _crewDutyAdj[c].size();
		for (int d = 0; d < duty_size; d++) {
			duty = (*_decidedDutySet)[d];			
			if (duty->startDtLoc - crew->workStatus->endDtLoc < _rules->horizon_rules->minOutRestMin) { //两个duty之间的间隔
				continue;
			}
			//若duty还未分配（==0）或部分分配（==1），则搜索还需分配的crew			
			int num_crew = duty->crewID.size();
			if (num_crew == 2) { continue; } //说明已经被分配完
			else if (num_crew == 1) {
				int c2 = duty->crewID.front();
				if (_crewMutualMatrix[c][c2] == 1) { //如果找不到符合的duty，即所有的duty都已包含一个与当前crew不match的crew
					//crew c可以担当duty d
					//update
					crew->workStatus->assigned = true;
					duty->crewID.push_front(c);

					_initialSoln->CrewDutyAdj[c].emplace_back(d);
					_initialSoln->DutyCrewAdj[d].emplace_back(c);
					_initialSoln->CrewDutyMatrix[c][d] = 1;

					//duty assigned over,update node of the duty
					for (auto& node : duty->route) {
						if (node->nodeType == NodeType::seg) {
							node->assigned = true;
						}
					}

					break;
				}
			}
			else if (num_crew == 0) {
				crew->workStatus->assigned = true;
				duty->crewID.push_front(c);

				_initialSoln->CrewDutyAdj[c].emplace_back(d);
				_initialSoln->DutyCrewAdj[d].emplace_back(c);
				_initialSoln->CrewDutyMatrix[c][d] = 1;
				break;
			}
		}

		c++;
		while (c < crew_size && (*_crewSet)[c]->workStatus->assigned == true) {
			c++;
		}
		//check stop
		finished = isCurSegCoverFinished();
	} while (c < crew_size && !finished);
	//感觉这样可能比较慢，因为是duty数基本是少于crew数。所以从CAP和FO交叉出发，选择duty

	if (finished == false) {	
		//debug
		_uncoveredFltFile = to_string(duty_size) + _uncoveredFltFile;
		_outStream.open(_uncoveredFltFile);
		int dd = 0;
		for (const auto& node : *_curSegNodeSet) {
			if (node->assigned == false) {				
				_outStream << _fltParser.toCsv(_fltParser.getDefaultHeaders(), node->segment) << "\n";
			}
		}
		_outStream.close();

		return 1; //说明有未覆盖
	}
	return 0;
}
void Assigner::initialSolution_baseCrewAndDay() {
	int crew_size = _crewSet->size();
	int total_duty_size = _decidedDutySet->size();
	_initialSoln = new Solution(crew_size, total_duty_size);
	/*_initialSoln->CrewDutyAdj.resize(crew_size);
	_initialSoln->DutyCrewAdj.resize(total_duty_size);*/

	for (auto& day_dutyIDs : _decidedDutyInDaysID) {
		auto curDecidedDutys = day_dutyIDs.second;
		initCrewDutyColumn(curDecidedDutys); //每天都需要初始化crew的状态：assign=false，更新dutyColumn
		setCrewDutyMatchAdj();

		int duty_size = curDecidedDutys.size();
		//Solution day_soln(crew_size, duty_size); //每天的解，求完每天的解，最后汇总得到当前迭代的解
		//day_soln.CrewDutyAdj.resize(crew_size);
		//day_soln.DutyCrewAdj.resize(duty_size);
		_initialSoln->CrewDutyAdj.clear();
		_initialSoln->DutyCrewAdj.clear();
		_initialSoln->CrewDutyAdj.resize(crew_size);
		_initialSoln->DutyCrewAdj.resize(total_duty_size);

		bool finished = true;
		int c = 0;
		int d = 0;
		CREW* crew;
		Path* duty;

		do {
			crew = (*_crewSet)[c];
			
			duty_size = _crewDutyAdj[c].size();
			for (int i = 0; i < duty_size; i++) {
				//d = curDecidedDutys[i];
				d = _crewDutyAdj[c][i];

				duty = (*_decidedDutySet)[d];
				
				if (duty->startDtLoc - crew->workStatus->endDtLoc < _rules->horizon_rules->minOutRestMin) { //两个duty之间的间隔
					continue;
				}
				//若duty还未分配（==0）或部分分配（==1），则搜索还需分配的crew			
				int num_crew = duty->crewID.size();
				if (num_crew == 2) { continue; } //说明已经被分配完
				else if (num_crew == 1) {
					int c2 = duty->crewID.front();
					if (_crewMutualMatrix[c][c2] == 1) { //如果找不到符合的duty，即所有的duty都已包含一个与当前crew不match的crew
						//crew c可以担当duty d
						//update

						if (d == 22 || d == 57) {
							int dd = 0;
						}
						if (crew->idCrew == "001551" || (*_crewSet)[c2]->idCrew == "001551") {
							int gg = 0;
						}
						

						crew->workStatus->assigned = true;
						duty->crewID.push_front(c);

						_initialSoln->CrewDutyAdj[c].emplace_back(d);
						_initialSoln->DutyCrewAdj[d].emplace_back(c);
						_initialSoln->CrewDutyMatrix[c][d] = 1;

						/////更新该duty上所有crew的状态
						updateCrewStatus(crew, duty);
						updateCrewStatus((*_crewSet)[c2], duty);
						/////

						//duty assigned over,update node of the duty
						for (auto& node : duty->route) {
							if (node->nodeType == NodeType::seg) {
								node->assigned = true;
							}
						}

						break;
					}
				}
				else if (num_crew == 0) {
					crew->workStatus->assigned = true;
					duty->crewID.push_front(c);

					if (crew->idCrew == "001551" /*|| (*_crewSet)[c2]->idCrew == "002798"*/) {
						int gg = 0;
					}

					_initialSoln->CrewDutyAdj[c].emplace_back(d);
					_initialSoln->DutyCrewAdj[d].emplace_back(c);
					_initialSoln->CrewDutyMatrix[c][d] = 1;
					break;
				}
			}

			c++;
			while (c < crew_size && (*_crewSet)[c]->workStatus->assigned == true) {
				c++;
			}
			//check single day stop,当天所有的duty是否安排完（未覆盖也算安排）
			for (const auto& dutyID : curDecidedDutys) {				
				if ((*_decidedDutySet)[dutyID]->crewID.size() < 2) {
					finished = false;
					break; 
				}
			}

		} while (c < crew_size && !finished);
		//感觉这样可能比较慢，因为是duty数基本是少于crew数。所以从CAP和FO交叉出发，选择duty

	}

	if (isCurSegCoverFinished() == false) {
		//debug
		/*_uncoveredFltFile = to_string(total_duty_size) + _uncoveredFltFile;
		_outStream.open(_uncoveredFltFile);
		int dd = 0;
		for (const auto& node : *_curSegNodeSet) {
			if (node->assigned == false) {
				_outStream << _fltParser.toCsv(_fltParser.getDefaultHeaders(), node->segment) << "\n";
			}
		}
		_outStream.close();*/
	}

}



bool Assigner::isCurSegCoverFinished() {
	for (const auto& node : *_curSegNodeSet) {
		if (node->assigned == false) {
			return false;
		}
	}
	return true;
}
void Assigner::updateCrewStatus(CREW* crew, Path* duty) {
	crew->workStatus->CreditedDutySet.emplace_back(duty);
	
	crew->workStatus->accumuFlyMin += duty->flyMin;

	int duration = std::abs(duty->startDtLoc - crew->workStatus->endDtLoc) / 60;
	if (crew->workStatus->restStation != "") {
		crew->workStatus->accumuCreditMin += duration; //初始天不需要加

		crew->workStatus->totalCreditMint += duration;
	}
	
	crew->workStatus->accumuCreditMin += duty->workMin;
	//判断是否间隔时长满足day off 7-18-23：53
	//added
	if (_rules->minDayOffMin < duration && duration <= 72 * 24 * 60) {
		crew->workStatus->accumuCreditMin = 0;
	}

	crew->workStatus->endDtLoc = duty->endDtLoc;
	crew->workStatus->restStation = duty->endStation;

	crew->workStatus->totalFlyMint += crew->workStatus->accumuFlyMin;
	crew->workStatus->totalCreditMint += duty->workMin;

}


void Assigner::calObjValue(Solution& solution) {
	//cal variance	
	updateCrewFlyMints(solution);
	
	int crew_size = solution.CrewDutyAdj.size();
	//1.variance 
	//2.dhd cost
	int total_flyMints = _initial_sumFlyMint;		
	
	for (int d = 0; d < solution.DutyCrewAdj.size(); d++) {
		if (solution.DutyCrewAdj[d].size() == 2) {
			total_flyMints += (*_decidedDutySet)[d]->flyMin;

			if ((*_decidedDutySet)[d]->route.front()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
			else if ((*_decidedDutySet)[d]->route.back()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
		}
	}	

	double mean_flyMint = total_flyMints / crew_size;

	for (int c = 0; c < crew_size; c++) {
		solution.obj.flyMintVariance += (_CrewFlyMints[c] - mean_flyMint)*(_CrewFlyMints[c] - mean_flyMint);
	}
	//solution.obj.flyMintVariance = std::sqrt(solution.obj.flyMintVariance);
	solution.obj.flyMintVariance /= (crew_size * crew_size);
	//3.flt cancel cost
	int uncoveredFlts = 0;
	for (const auto& seg : *_curSegNodeSet) {
		if (seg->assigned == false) {
			++uncoveredFlts;
		}
	}
	solution.obj.fltCancelCost = uncoveredFlts;
	
	//sum
	solution.obj.objValue = solution.obj.flyMintVariance + solution.obj.fltCancelCost + solution.obj.dhdCost;
}
void Assigner::updateCrewFlyMints(Solution& solution) {
	int d = 0;
	for (int c = 0; c < solution.CrewDutyAdj.size(); c++) {
		for (int j = 0; j < solution.CrewDutyAdj[c].size(); j++) { //crew应该只有一个duty，实际应该size() == 1
			d = solution.CrewDutyAdj[c][j];
			_CrewFlyMints[c] += (*_decidedDutySet)[d]->flyMin;
		}
	}
} 


/*----------post-process----------*/
void Assigner::postProcess() {
	//根据optSoln来赋值
	_assignedDutySet = new std::vector<Path*>();
	int num_decidedDuty = _decidedDutySet->size();
	_assignedDutySet->reserve(num_decidedDuty);
		
	CREW* crew;
	Path* duty;
	for (int d = 0; d < num_decidedDuty; d++) {
		if (_optSoln->DutyCrewAdj[d].size() == 2) { //完全覆盖（crew人数达标）的duty才算覆盖
			duty = (*_decidedDutySet)[d];
			_assignedDutySet->emplace_back(duty);

			//update crew workstatus
			//for (const auto& crew_index : _optSoln->DutyCrewAdj[d]) {
			//	crew = (*_crewSet)[crew_index];
			//	
			//	crew->workStatus->CreditedDutySet.emplace_back(duty);

			//	if (crew->idCrew == "003537"
			//		&& crew->workStatus->CreditedDutySet.size() > 0) {
			//		int uuuu = 0;

			//	}
			//	
			//	std::sort(crew->workStatus->CreditedDutySet.begin(), crew->workStatus->CreditedDutySet.end(),
			//		[](const Path* a, const Path* b) {return a->endDtLoc < b->endDtLoc; });
			//	
			//					

			//	crew->workStatus->accumuFlyMin += duty->flyMin;
			//	if (crew->workStatus->restStation != "") { 
			//		crew->workStatus->accumuCreditMin += std::abs(duty->startDtLoc - crew->workStatus->endDtLoc) / 60; //初始天不需要加
			//	}				
			//	crew->workStatus->accumuCreditMin += duty->workMin;
			//	
			//	/*crew->workStatus->endDtLoc = duty->endDtLoc;
			//	crew->workStatus->restStation = duty->endStation;*/
			//	crew->workStatus->endDtLoc = crew->workStatus->CreditedDutySet.back()->endDtLoc;
			//	crew->workStatus->restStation = crew->workStatus->CreditedDutySet.back()->endStation;

			//	
			//	crew->workStatus->totalFlyMint += crew->workStatus->accumuFlyMin;
			//	crew->workStatus->totalCreditMint += crew->workStatus->accumuCreditMin;
			//}
		}
	}	
}

/*----------output-process----------*/
std::vector<Path*>* Assigner::getAssignedDutySet() {
	
	return _assignedDutySet;
}


