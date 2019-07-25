#include "Assignment.h"

//Solution::Solution(const int numRow, const int numColumn) {	
	//CrewDutyMatrix.resize(numRow);
	//for (size_t i = 0; i < numRow; i++) {
	//	CrewDutyMatrix[i].resize(numColumn);
	//}	
	////adj
	//CrewDutyAdj.clear();
	//DutyCrewAdj.clear();
	//CrewDutyAdj.resize(numRow);
	//DutyCrewAdj.resize(numColumn);
//}


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
void Assigner::init(std::vector<CREW*>* p_crewSet, 
	CrewRules* p_rules, 
	std::map<std::string, posOrderSeqVec*>* p_pos_order_seqs,
	std::map<int, orderPosSeqVec*>* p_order_pos_seqs,
	std::vector<CREW*>* p_curCAPCrewSet,
	std::vector<CREW*>* p_curFOCrewSet,
	std::vector<CREW*>* p_curAttendantCrewSet) {
	_crewSet = p_crewSet;
	_rules = p_rules;
	_pos_order_seqs_ptr = p_pos_order_seqs;
	_order_pos_seqs_ptr = p_order_pos_seqs;

	_curCAPCrewSet = p_curCAPCrewSet;
	_curFOCrewSet = p_curFOCrewSet;
	_curAttendantCrewSet = p_curAttendantCrewSet;

	for (auto crew : *_crewSet) {
		if (_crewDutyPairs.find(crew) == _crewDutyPairs.end()) {
			_crewDutyPairs[crew] = std::vector<Path*>();
		}		
	}
}
void Assigner::receiveSetCoverInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet) {	
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
void Assigner::initRankCompositionMap() {
	/*for (const auto& seq : _rules->rankCombinationSet) {
		_requireNum_pos_seq[]
	}*/
	
}

//����ָ��ִ��һ�μ���
void Assigner::clusterDutyByDay() {	
	//��_decidedDutySet��ִ��ʱ�併������
	//����֮�����dutyҲ���Ѿ��ź�����
	std::sort(_decidedDutySet->begin(), _decidedDutySet->end()
		, [](const Path* a, const Path* b) {return a->workMin > b->workMin; });

	//_decidedDutyInDaysID.clear();	
	Path* duty;
	for (int d = 0; d < _decidedDutySet->size(); d++) {
		duty = (*_decidedDutySet)[d];
		//_decidedDutyInDaysID[duty->startDate].emplace_back(d); //��ŵ���index
		_decidedDutyInDays[duty->startDate].emplace_back(duty);
	}		
}
void Assigner::initFlyMints() {
	_initial_sumFlyMint = 0;

	/*_CrewFlyMints.clear();
	_CrewFlyMints.reserve(_crewSet->size());	
	for (const auto& crew : *_crewSet) {
		_initial_sumFlyMint += crew->workStatus->accumuFlyMin;
		_CrewFlyMints.emplace_back(crew->workStatus->accumuFlyMin);
	}*/

	for (const auto& crew : *_crewSet) {
		_initial_sumFlyMint += crew->workStatus->accumuFlyMin;
		_CrewFlyMintsMap[crew] = crew->workStatus->accumuFlyMin;
	}
}

//ִ�д��� = dutySet����������
void Assigner::initCrewDutyColumn(/*std::vector<int>& decidedDutysID*/std::vector<Path*>& decidedDutys) {
	for (auto& crew : *_crewSet) {
		crew->workStatus->assigned = false;
		//crew->workStatus->setDutyColumn(_decidedDutySet->size());
	}
	for (auto& c_d : _crewDutyPairs) {
		c_d.second.clear();
	}
	for (auto& d_c : _dutyCrewPairs) {
		d_c.second.clear();
	}

	int crew_size = _crewSet->size();	
	int duty_size = decidedDutys.size();

	//CREW* temp_crew;
	int gap_crew_duty = 0;//crew��ǰʱ������������duty��ʼʱ��ļ��
	bool exist_feasible_duty;
	for (auto temp_crew = _crewSet->begin(); temp_crew != _crewSet->end(); temp_crew++) {		
		exist_feasible_duty == false;

		if ((*temp_crew)->workStatus->accumuCreditMin > _rules->maxCreditMin ||
			(*temp_crew)->workStatus->accumuFlyMin > _rules->maxWeekFlyMin) {
			//need a day off
			(*temp_crew)->workStatus->accumuCreditMin = 0;
			(*temp_crew)->workStatus->assigned = true; //assigned a day off
			(*temp_crew)->workStatus->endDtLoc += _rules->minDayOffMin;
			(*temp_crew)->workStatus->accumuFlyMin = 0;
		}
		else  { //ִ��,����ʱ��С�ڹ涨�����
			Path* duty;
			int d = 0;
			for (int i = 0; i < duty_size; i++) {				
				duty = decidedDutys[i];

				//1.���ռ�����Ƿ�����
				if ((*temp_crew)->workStatus->restStation != "" && (*temp_crew)->workStatus->restStation != duty->startStation) {
					//=="",˵���ǳ��ε���������Ҫ����ռ����������ʵ����crew�ĳ�ʼ״̬������һ���ƻ����ڿ�ʼʱ�����ڵص���Ϣ�������Ȳ�����
					continue;
				}								
				//2.���ʱ��Լ��
				gap_crew_duty = (duty->startDtLoc - (*temp_crew)->workStatus->endDtLoc) / 60;
				if (gap_crew_duty <= 0) {
					continue;
				}
				else if (gap_crew_duty > 72 * 24 * 60) {
					break;
				}
				else if (0 < gap_crew_duty && gap_crew_duty < _rules->minDayOffMin) {
					//����Ҫday off������duty֮�����������
					//��Ԥ���ж�һ����������duty���Ƿ�ᳬʱ
					if ((*temp_crew)->workStatus->accumuCreditMin + gap_crew_duty + duty->workMin > _rules->maxCreditMin
						|| (*temp_crew)->workStatus->accumuFlyMin + duty->flyMin > _rules->maxWeekFlyMin) {
						break;
					}
				}
				//���ܵ�����duty����Ϊ�ᳬʱ.dԽ��ʱ��Խ�������Բ��������²��ң�ֱ��break				

				//3.�����ռ��ʱ�䣬�������
				string special = "SpecialAirport";
				if (duty->specialCredentials[special] >= 1 &&
					(*temp_crew)->rankAry->front()->SkillSet[special] != 1) { //duty��Ҫ�������ʣ���crew���߱�
					continue;
				}
				//�����㣬��duty�͸�crew���Դ���
				exist_feasible_duty = true;				
				_crewDutyPairs[*temp_crew].emplace_back(duty);
				_dutyCrewPairs[duty].emplace_back(*temp_crew);
			}		
		}	
		//��ǰ���Ҳ����ɵ�����duty����һ����Ҫday off			
		if (exist_feasible_duty == false) {			
			(*temp_crew)->workStatus->assigned = true; //assigned a break
		}
	}
	//sort _curCrewSet��ִ��ʱ����������
	/*std::sort(_crewSet->begin(), _crewSet->end()
		, [](const CREW* a, const CREW* b) {return a->workStatus->accumuCreditMin < b->workStatus->accumuCreditMin; });*/
	
	Path* duty;
	for (int d = 0; d < decidedDutys.size(); d++) {
		duty = decidedDutys[d];
		if (_dutyCrewPairs.find(duty) == _dutyCrewPairs.end()) {
			std::cout << "uncover duty: " << d << std::endl;
		}
	}


	sortCrewSet();
}
//void Assigner::setCrewDutyMatchAdj() {
//	_dutyCrewAdj.clear();
//	_crewDutyAdj.clear(); 
//	_dutyCrewAdj.shrink_to_fit();
//	_crewDutyAdj.shrink_to_fit();
//
//	int crew_size = _crewSet->size();
//	int duty_size = _decidedDutySet->size();
//	_dutyCrewAdj.resize(duty_size);
//	_crewDutyAdj.resize(crew_size);
//
//	for (int c = 0; c < crew_size; c++) {
//		for (int d = 0; d < duty_size; d++) {
//			if ((*_crewSet)[c]->workStatus->dutyColumn[d] == 1) {
//				_dutyCrewAdj[d].emplace_back(c);
//
//				_crewDutyAdj[c].emplace_back(d);
//			}
//		}
//	}
//}

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


/*----------algorithm-process----------*/

///initialSolution by duty to find crew
//void Assigner::initialSolution() {
//	_initialSoln = new Solution(_crewSet->size(), _decidedDutySet->size());
//	_initialSoln->CrewDutyAdj.resize(_crewSet->size());
//	_initialSoln->DutyCrewAdj.resize(_decidedDutySet->size());
//
//	//debug
//	Path* path;
//	for (int i = 0;i < _decidedDutySet->size();i++) {
//		path = (*_decidedDutySet)[i];
//		for (const auto& node : path->route) {
//			if (node->nodeType == NodeType::seg) {
//				if (node->segment->getDBId() == 54508631
//					|| node->segment->getDBId() == 54508237
//					|| node->segment->getDBId() == 54503251) {
//					int ddd = 0;
//				}
//			}
//			
//		}
//	}
//
//
//	int duty_size = _decidedDutySet->size();
//	bool finished = true;
//	bool duty_assigned = false;
//	int c1 = 0, c2 = 0;
//	size_t d = 0;
//	do {
//		int i = 0;
//		int crew_size = _dutyCrewAdj[d].size();
//		duty_assigned = false;
//		while (i < crew_size && duty_assigned == false) {
//			c1 = _dutyCrewAdj[d][i];
//			if ((*_crewSet)[c1]->workStatus->assigned == true) {
//				++i;
//				continue;
//			}
//
//			for (int j = i + 1; j < crew_size; j++) {
//				c2 = _dutyCrewAdj[d][j];
//				//if not mutual and have not be assigned to some duty
//				if (
//					(*_crewSet)[c2]->workStatus->assigned == false) { 
//					
//					(*_crewSet)[c1]->workStatus->assigned = true;
//					(*_crewSet)[c2]->workStatus->assigned = true;
//					/*(*_decidedDutySet)[d]->work_crewSet.emplace_back((*_crewSet)[c1]);
//					(*_decidedDutySet)[d]->work_crewSet.emplace_back((*_crewSet)[c2]);*/
//
//					_initialSoln->CrewDutyMatrix[c1][d] = 1;
//					_initialSoln->CrewDutyMatrix[c2][d] = 1;
//					_initialSoln->CrewDutyAdj[c1].emplace_back(d);
//					_initialSoln->CrewDutyAdj[c2].emplace_back(d);
//					_initialSoln->DutyCrewAdj[d].emplace_back(c1);
//					_initialSoln->DutyCrewAdj[d].emplace_back(c2);
//
//
//					for (auto& node : (*_decidedDutySet)[d]->route) {
//						if (node->nodeType == NodeType::seg) { 
//							node->assigned = true; 
//						}
//					}
//
//					duty_assigned = true;
//					break;
//				}
//			}
//			i++;
//		}
//		
//		//check if all node in _curSegNodeSet were assigned		
//		finished = true;
//		finished = isCurSegCoverFinished();
//		
//		d++;
//		if (d == 80 || d == 95 || d == 100) {
//			int mmmm = 0;
//		}
//
//	} while (d < duty_size && finished == false);
//	//if not feasible?
//	if (finished == false) {
//		//debug
//		_uncoveredFltFile = to_string(duty_size) + _uncoveredFltFile;
//		_outStream.open(_uncoveredFltFile);
//		int dd = 0;
//		for (const auto& node : *_curSegNodeSet) {
//			if (node->assigned == false) {
//				//std::cout << "id<" << node->segment->getDBId() << ">, fltNum<" << node->segment->getFlightNumber() << ">\n";
//				_outStream << _fltParser.toCsv(_fltParser.getDefaultHeaders(), node->segment) << "\n";
//			}
//		}
//		_outStream.close();
//	}
//}

void Assigner::initialSolution_baseCrewAndDay() {	
	_initialSoln = new Solution();
	
	for (auto& day_dutys : _decidedDutyInDays) {
		auto curDecidedDutys = day_dutys.second;
		initCrewDutyColumn(curDecidedDutys); //ÿ�춼��Ҫ��ʼ��crew��״̬��assign=false������dutyColumn		
		int duty_size = curDecidedDutys.size();				

		bool finished = true;
		CREW* crew;
		Path* duty;
		string rank("");
		auto iter = _crewSet->begin();//_crewDutyPairs.begin();
		auto end = _crewSet->end();//_crewDutyPairs.end();
		while (iter != end && !finished) {
			crew =  *iter;	
			duty_size = _crewDutyPairs[crew].size();
			for (int i = 0; i < duty_size; i++) {								
				duty = _crewDutyPairs[crew][i];
				
				if (duty->crewCombination.size() > 3) {
					int yyy = 0;
				}

				//1.����duty֮��ļ�������㣬���duty�Ѿ��������
				if (duty->startDtLoc - crew->workStatus->endDtLoc < _rules->horizon_rules->minOutRestMin
					|| duty->assigned == true) { 
					continue;
				}
				rank = crew->rankAry->front()->rank;
				//��duty��Ӧ��rank�Ѿ������꣬����һ��crew��rank�����Է����duty
				if (duty->tempCompoMode.rankNum[rank] == 0) {
					continue;
				}
				
				//�ȷ��䣬���ж�
				//���·���ķ����				
				duty->crewCombination.emplace_back(crew); 				
				int num_crew = duty->crewCombination.size();		
				//2.==1,���������Լ��;���򣬼��
				//����Լ�������㣬ѡ��һ��duty
				if (num_crew > 1 && !isFasibleCombination(duty->crewCombination)) { 					
					duty->crewCombination.pop_back();
					continue;
				}				
				else {
					crew->workStatus->assigned = true;
					//update duty's tempCompoNums
					try {
						updateDutyRankNums(duty, rank);
					}
					catch (const char& msg) {
						std::cerr << msg << std::endl;
					}
					
					_initialSoln->CrewDutyPairs[crew].emplace_back(duty);
					_initialSoln->DutyCrewPairs[duty].emplace_back(crew);
					
					//����duty���Ƿ�������
					if (/*duty->crewCombination.size() == duty->compoNums->totalNum*/
						duty->tempCompoMode.totalNum == 0) {
						duty->assigned = true;
						//����node,��ʵ���Բ�����node��ֻ��Ҫ�жϵ����duty�Ƿ�����꣬��ô��Ȼ����node��������
						for (auto& node : duty->route) {
							if (node->nodeType == NodeType::seg) {
								node->assigned = true;
							}
						}
					}
					//��crew������ϣ���һ��crew��ѡduty
					break;
				}				
			}

			iter++;
			while (iter != end && (*iter)->workStatus->assigned == true) {
				iter++;
			}
			finished = true;			
			//check single day stop,�������е�duty�Ƿ�����
			//��ǰ����
			for (const auto& cur_duty : curDecidedDutys) {				
				if (cur_duty->assigned == false) {
					finished = false;
					break; 
				}
			}

		} 		
	}

	if (isCurSegCoverFinished() == false) {
		Path* duty;
		for (int i = 0; i < _decidedDutySet->size(); i++) {
			duty = (*_decidedDutySet)[i];
			if (duty->assigned == false) {
				std::cout << "unassigned duty: " << i << std::endl;
			}
		}
		
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
bool Assigner::isFasibleCombination(const std::vector<CREW*>& crewComb) {	
	CREW* crew;
	std::vector<string> combi;
	for (size_t i = 0; i < crewComb.size(); i++) {
		crew = crewComb[i];
		combi.emplace_back(crew->rankAry->front()->position);
	}
	//std::sort(combi.begin(), combi.end());
	//�����󽻼�
	seqIdVec intersection;
	seqIdVec *p_set1 = &(*_pos_order_seqs_ptr)[combi[0]]->orderSeqIdvec[0];
	seqIdVec *p_set2;
	for (int i = 1; i < combi.size(); i++) {
		p_set2 = &(*_pos_order_seqs_ptr)[combi[i]]->orderSeqIdvec[i];		
		std::set_intersection(p_set1->begin(), p_set1->end(), p_set2->begin(), p_set2->end(),
			std::insert_iterator<seqIdVec>(intersection, intersection.begin()));		
		
		p_set1 = &intersection;		
	}

	return intersection.size() != 0;
}

void Assigner::updateDutyRankNums(Path* duty, const string& rank) {
	--duty->tempCompoMode.totalNum;
	if (duty->tempCompoMode.rankNum.find(rank) == duty->tempCompoMode.rankNum.end()) {
		throw "rank is not the key of duty.compoMode.rankNum!";
	}
	--duty->tempCompoMode.rankNum[rank];
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
		crew->workStatus->accumuCreditMin += duration; //��ʼ�첻��Ҫ��

		crew->workStatus->totalCreditMint += duration;
	}
	
	crew->workStatus->accumuCreditMin += duty->workMin;
	//�ж��Ƿ���ʱ������day off 7-18-23��53
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
	
	//int crew_size = solution.CrewDutyAdj.size();
	int crew_size = solution.CrewDutyPairs.size();
	//1.variance 
	//2.dhd cost
	int total_flyMints = _initial_sumFlyMint;		
	
	/*for (int d = 0; d < solution.DutyCrewAdj.size(); d++) {
		if (solution.DutyCrewAdj[d].size() == 2) {
			total_flyMints += (*_decidedDutySet)[d]->flyMin;
			if ((*_decidedDutySet)[d]->route.front()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
			if ((*_decidedDutySet)[d]->route.back()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
		}
	}*/	
	for (const auto& d_c : solution.DutyCrewPairs) {
		if (d_c.first->assigned) {
			total_flyMints += d_c.first->flyMin;
			if (d_c.first->route.front()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
			if (d_c.first->route.back()->nodeType == NodeType::dhd) { solution.obj.dhdCost += 1; }
		}
	}

	double mean_flyMint = total_flyMints / crew_size;
	double sum_square = 0;
	/*for (int c = 0; c < crew_size; c++) {
		solution.obj.flyMintVariance += (_CrewFlyMints[c] - mean_flyMint)*(_CrewFlyMints[c] - mean_flyMint);		
	}*/
	for (const auto& crew_flymin : _CrewFlyMintsMap) {
		sum_square += std::pow(crew_flymin.second - mean_flyMint, 2);
	}

	//solution.obj.flyMintVariance = std::sqrt(solution.obj.flyMintVariance);
	solution.obj.flyMintVariance = sum_square / (crew_size * crew_size);
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
	//for (int c = 0; c < solution.CrewDutyAdj.size(); c++) {
	//	for (int j = 0; j < solution.CrewDutyAdj[c].size(); j++) { //crewһ��Ӧ��ֻ��һ��duty��������ж��duty
	//		d = solution.CrewDutyAdj[c][j];
	//		_CrewFlyMints[c] += (*_decidedDutySet)[d]->flyMin;
	//	}
	//}
	for (const auto& c_d : solution.CrewDutyPairs) {
		for (const auto& duty : c_d.second) {
			_CrewFlyMintsMap[c_d.first] += duty->flyMin;
		}		
	}
} 



/*----------post-process----------*/
void selectDayoffCrew(/*std::vector<CREW*>& crewSet*/) {


}



void Assigner::postProcess() {
	//����optSoln����ֵ
	_assignedDutySet = new std::vector<Path*>();
	int num_decidedDuty = _decidedDutySet->size();
	_assignedDutySet->reserve(num_decidedDuty);
		
	CREW* crew;
	Path* duty;
	/*for (int d = 0; d < num_decidedDuty; d++) {		
		if (_optSoln->DutyCrewAdj[d].size() == 2) {
			duty = (*_decidedDutySet)[d];
			_assignedDutySet->emplace_back(duty);
		}
	}*/

	for (const auto& k_v : _optSoln->DutyCrewPairs) {
		if (k_v.first->assigned) {
			_assignedDutySet->emplace_back(k_v.first);
		}
	}
}

/*----------output-process----------*/
std::vector<Path*>* Assigner::getAssignedDutySet() {
	
	return _assignedDutySet;
}


