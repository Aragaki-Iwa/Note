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
}

int Assigner::solve() {	
	//initialization
	initFlyMint();	
	initCrewDutyColumn();
	initMatrixs();
	//optimization
	//initialSolution();
	//initialSolution_basedCrew();
	initialSolution_baseCrewAndDay();
	
	calObjValue(*_initialSoln);

	_optSoln = _initialSoln;
	postProcess();

	std::cout << "opt solution's uncovered flt: " << _optSoln->obj.fltCancelCost << "\n";

	std::cout << "from _decidedDutySet which inlude " << _decidedDutySet->size() << " num of dutys\n";
	std::cout << "got _assignedDutySet which inlude " << _assignedDutySet->size() << " num of dutys\n";

	return 1;
}
/*public, for init*/
void Assigner::init(std::vector<CREW*>* p_crewSet, CrewRules* p_rules,
	std::vector<CREW*>* p_curCAPCrewSet,
	std::vector<CREW*>* p_curFOCrewSet,
	std::vector<CREW*>* p_curAttendantCrewSet) {
	_crewSet = p_crewSet;
	_rules = p_rules;
	_curCAPCrewSet = p_curCAPCrewSet;
	_curFOCrewSet = p_curFOCrewSet;
	_curAttendantCrewSet = p_curAttendantCrewSet;

}
void Assigner::receiveInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet) {
	
	_decidedDutySet = p_decidedDutySet;	
	_curSegNodeSet = p_curSegNodeSet;
	
}
void Assigner::labelDecidedDuty(std::vector<string>& specialAirport) {
	int duty_size = _decidedDutySet->size();
	Path* duty;
	//���ҳ��������ʵ�duty,Ŀǰֻ���������Ҫ��
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

	//��dutyʱ����������ִ��ʱ
	std::sort(_decidedDutySet->begin(), _decidedDutySet->end()
		, [](const Path* a, const Path* b) {return a->workMin > b->workMin; });

}

/*private*/
void Assigner::clusterDutyByDay() {	
	//std::sort(_decidedDutySet->begin(), _decidedDutySet->end()
	//	, [](const Path* a, const Path* b) {return a->startDtLoc > b->startDtLoc; });//����ʼʱ����������	
	////�ҵ�first��last day�������õ�_decidedDutySet�����м����duty
	//time_t first_day = getStartTimeOfDay(_decidedDutySet->front()->startDtLoc);
	//time_t last_day = getStartTimeOfDay(_decidedDutySet->back()->startDtLoc);
	//int num_days = (last_day - first_day) / (24 * 3600) + 1;
	////_decidedDutySet�е�duty�ɷ�Ϊnum_days��
	//_decidedDutyInDays.resize(num_days);
	//Path* duty;
	//time_t duty_startDay;
	//for (int d = 0; d < _decidedDutySet->size(); d++) {
	//	duty = (*_decidedDutySet)[d];
	//	duty_startDay = getStartTimeOfDay(duty->startDtLoc);
	//}

	for (const auto& duty : *_decidedDutySet) {
		_decidedDutyInDays[duty->startDate].emplace_back(duty);
	}
	//�����duty�ֱ�ִ��ʱ����������
	for (auto& day_dutySet : _decidedDutyInDays) {
		std::sort(day_dutySet.second.begin(), day_dutySet.second.end()
			, [](const Path* a, const Path* b) {return a->startDtLoc > b->startDtLoc; });
	}	

}

void Assigner::initCrewDutyColumn(std::vector<Path*>& decidedDutys) {
	for (auto& crew : *_crewSet) {
		crew->workStatus->assigned = false;
	}

	int crew_size = _crewSet->size();
	//int duty_size = _decidedDutySet->size(); //_decidedDutySet��ΪdecidedDutys 7-18
	int duty_size = decidedDutys.size();

	//CREW* temp_crew;
	bool exist_feasible_duty = false;
	for (/*int c = 0; c < crew_size; c++*/
		auto temp_crew = _crewSet->begin(); temp_crew != _crewSet->end(); ) {
		//temp_crew = crewSet[c];
		(*temp_crew)->workStatus->setDutyColumn(duty_size);
		 
		if ((*temp_crew)->workStatus->accumuCreditMin < _rules->maxCreditMin) {
			Path* duty;			
			for (int d = 0; d < duty_size; d++) {
				//duty = (*_decidedDutySet)[d];
				duty = decidedDutys[d];

				if ((*temp_crew)->workStatus->restStation != "" && (*temp_crew)->workStatus->restStation != duty->startStation) {
					//=="",˵���ǳ��ε���������Ҫ����ռ����������ʵ����crew�ĳ�ʼ״̬������һ���ƻ����ڿ�ʼʱ�����ڵص���Ϣ�������Ȳ�����
					continue;
				}
				//��duty�ļ��������minDayOffMint,˵���ڵ�����dutyǰ��ʵ���ǽ�����һ��day off
				if ((*temp_crew)->workStatus->accumuCreditMin + duty->workMin >= _rules->maxCreditMin
					|| (*temp_crew)->workStatus->accumuCreditMin + duty->workMin + _rules->allowOverCreditMin >= _rules->maxCreditMin) {
					break; //���ܵ�����duty����Ϊ�ᳬʱ.dԽ��ʱ��Խ�������Բ��������²��ң�ֱ��break
				}

				exist_feasible_duty = true;
				(*temp_crew)->workStatus->dutyColumn[d] = 1; //�Ȳ��������ʣ�ֻ���ǿռ��ʱ�䣬����crew�����Ե�������duty

			}		
		}

		if (exist_feasible_duty == false) {
			(*temp_crew)->workStatus->dayoffMin = _rules->minDayOffMin;
			(*temp_crew)->workStatus->inDay += 2; //day off 2 days
			(*temp_crew)->workStatus->accumuCreditMin = 0;
			(*temp_crew)->workStatus->accumuRestMin += _rules->minDayOffMin;
			
			(*temp_crew)->workStatus->endDtLoc += _rules->minDayOffMin;

			(*temp_crew)->workStatus->assigned = true; //assigned a day off
		}
		else {
			//�������ʣ�special dutyֻ��special crew���Ե���
			Path* duty;
			bool exist_feasible_duty = false;
			for (int d = 0; d < duty_size; d++) {
				//duty = (*_decidedDutySet)[d];
				duty = decidedDutys[d];
				string special = "SpecialAirport";
				if (duty->specialCredentials[special] >= 1 &&
					(*temp_crew)->rankAry->front()->SkillSet[special] != 1) {
					(*temp_crew)->workStatus->dutyColumn[d] = 0;
				}
			}

			temp_crew++;
		}			
	}
	//sort _curCrewSet��ִ��ʱ����������
	/*std::sort(_crewSet->begin(), _crewSet->end()
		, [](const CREW* a, const CREW* b) {return a->workStatus->accumuCreditMin < b->workStatus->accumuCreditMin; });*/
	sortCrewSet();
}
void Assigner::sortCrewSet() {
	/*�����CAP ��FO������ѡduty*/
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

void Assigner::initFlyMint() {
	_initial_sumFlyMint = 0;
	_CrewFlyMints.reserve(_crewSet->size());
	for (const auto& crew : *_crewSet) {
		_initial_sumFlyMint += crew->workStatus->accumuFlyMin;
		_CrewFlyMints.emplace_back(crew->workStatus->accumuFlyMin);
	}
}
void Assigner::initMatrixs() {
	int crew_size = (*_crewSet).size();
	int duty_size = _decidedDutySet->size();
	
	_crewMutualMatrix.clear();
	_dutyCrewAdj.clear();
	_crewDutyAdj.clear();

	_crewMutualMatrix.shrink_to_fit();
	_dutyCrewAdj.shrink_to_fit();
	_crewDutyAdj.shrink_to_fit();

	_crewMutualMatrix.resize(crew_size);
	_dutyCrewAdj.resize(duty_size);
	_crewDutyAdj.resize(crew_size);
	/*crew mutual*/
	for (int i = 0; i < crew_size; i++) {
		_crewMutualMatrix[i].resize(crew_size, 1);
	}
	//set value for mutual crews	
	for (int i = 0; i < crew_size; i++) {
		_crewMutualMatrix[i][i] = 0; //diagonal = 0
		for (int j = 0; j < crew_size; j++) {
			if ((*_crewSet)[i]->rankAry->front()->rank == (*_crewSet)[j]->rankAry->front()->rank) {
				_crewMutualMatrix[i][j] = 0; //same rank
			}
			else if (!isRankMatch((*_crewSet)[i]->rankAry->front(), (*_crewSet)[j]->rankAry->front())) {
				_crewMutualMatrix[i][j] = 0; //rank combination mutual
			}
		}
	}

	/*duty-crewSet*/ //adjective table
	for (int c = 0; c < crew_size; c++) {
		for (int d = 0; d < duty_size; d++) {
			if ((*_crewSet)[c]->workStatus->dutyColumn[d] == 1) {
				_dutyCrewAdj[d].emplace_back(c);

				_crewDutyAdj[c].emplace_back(d);
			}
		}
	}

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
		finished = stop();
		
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
			if (duty->startDtLoc - crew->workStatus->endDtLoc < _rules->horizon_rules->minOutRestMin) { //����duty֮��ļ��
				continue;
			}
			//��duty��δ���䣨==0���򲿷ַ��䣨==1������������������crew			
			int num_crew = duty->crewID.size();
			if (num_crew == 2) { continue; } //˵���Ѿ���������
			else if (num_crew == 1) {
				int c2 = duty->crewID.front();
				if (_crewMutualMatrix[c][c2] == 1) { //����Ҳ������ϵ�duty�������е�duty���Ѱ���һ���뵱ǰcrew��match��crew
					//crew c���Ե���duty d
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
		finished = stop();
	} while (c < crew_size && !finished);
	//�о��������ܱȽ�������Ϊ��duty������������crew�������Դ�CAP��FO���������ѡ��duty

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

		return 1; //˵����δ����
	}
	return 0;
}
void Assigner::initialSolution_baseCrewAndDay() {
	clusterDutyByDay();
	int crew_size = _crewSet->size();
	for (auto& day_duty : _decidedDutyInDays) {
		auto curDecidedDutys = day_duty.second;
		initCrewDutyColumn(curDecidedDutys); //ÿ�춼��Ҫ��ʼ��crew��״̬��assign=false������dutyColumn
		
		int duty_size = curDecidedDutys.size();
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
				duty = curDecidedDutys[d];
				
				if (duty->startDtLoc - crew->workStatus->endDtLoc < _rules->horizon_rules->minOutRestMin) { //����duty֮��ļ��
					continue;
				}
				//��duty��δ���䣨==0���򲿷ַ��䣨==1������������������crew			
				int num_crew = duty->crewID.size();
				if (num_crew == 2) { continue; } //˵���Ѿ���������
				else if (num_crew == 1) {
					int c2 = duty->crewID.front();
					if (_crewMutualMatrix[c][c2] == 1) { //����Ҳ������ϵ�duty�������е�duty���Ѱ���һ���뵱ǰcrew��match��crew
						//crew c���Ե���duty d
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
			finished = stop();
		} while (c < crew_size && !finished);
		//�о��������ܱȽ�������Ϊ��duty������������crew�������Դ�CAP��FO���������ѡ��duty

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
		}

	}

}



bool Assigner::isRankMatch(CREW_RANK* cap, CREW_RANK* fo) {
	std::string combination;
	if (cap->rank == "CAP") { combination = cap->position + "-" + fo->position; }
	else if (cap->rank == "FO") { combination = fo->position + "-" + cap->position; }
		
	return _rules->rankCombinationSet.find(combination) != _rules->rankCombinationSet.end();
}
bool Assigner::stop() {
	for (const auto& node : *_curSegNodeSet) {
		if (node->assigned == false) {
			return false;
		}
	}
	return true;
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
		for (int j = 0; j < solution.CrewDutyAdj[c].size(); j++) { //crewӦ��ֻ��һ��duty��ʵ��Ӧ��size() == 1
			d = solution.CrewDutyAdj[c][j];
			_CrewFlyMints[c] += (*_decidedDutySet)[d]->flyMin;
		}
	}
} 


/*----------post-process----------*/
void Assigner::postProcess() {
	//����optSoln����ֵ
	_assignedDutySet = new std::vector<Path*>();
	int num_decidedDuty = _decidedDutySet->size();
	_assignedDutySet->reserve(num_decidedDuty);
		
	CREW* crew;
	Path* duty;
	for (int d = 0; d < num_decidedDuty; d++) {
		if (_optSoln->DutyCrewAdj[d].size() == 2) { //��ȫ���ǣ�crew������꣩��duty���㸲��
			duty = (*_decidedDutySet)[d];
			_assignedDutySet->emplace_back(duty);

			//update crew workstatus
			for (const auto& crew_index : _optSoln->DutyCrewAdj[d]) {
				crew = (*_crewSet)[crew_index];
				
				crew->workStatus->accumuFlyMin += duty->flyMin;
				crew->workStatus->accumuCreditMin += (duty->startDtLoc - crew->workStatus->endDtLoc);				
				crew->workStatus->accumuCreditMin += duty->workMin;
				crew->workStatus->endDtLoc = duty->endDtLoc;
				crew->workStatus->restStation = duty->endStation;
			}
		}
	}	
}

/*----------output-process----------*/
std::vector<Path*>* Assigner::getAssignedDutySet() {
	
	return _assignedDutySet;
}

