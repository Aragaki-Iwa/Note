#include "Optimizer.h"
#include "..\csvReader\UtilFunc.h"
#include "csvClassesTransOptimizable.h"
#include "CrewRules.h"
#include "Seg_Network.h"
#include "Crew_Network.h"
#include "Crew_Path.h"
#include "Seg_Path.h"
#include "CrewGroupSearcher.h"
#include "OutputHandler.h"

//! for debug
#include "SummeryTool_H.h"
static Summery::StopWatch TIMER;

const char* CREW_STATUS_FILE = "../data/output/crew_status_record.txt";


//! end for debug

Optimizer::Optimizer() {	
	//_column_generation = new ColumnGeneration();
}
Optimizer::~Optimizer() {
	delete _crewNet;
	delete _segNet;	
}


void Optimizer::optimize() {
	// 1.initialize	
	_segNet->createNetwork();
	
	TIMER.Start();	
	_crewNet->createNetwork();
	TIMER.Stop();
	TIMER.printElapsed();

	OutputHandler output_handler;
	std::stringstream cur_day_sch_file;
	std::stringstream cur_day_crew_status_file;

	
	_begining_plan = getStartTimeOfDay(_segNet->nodeSet.front()->startDtLoc);
	for (auto& crew : _optCrewSet) {
		crew->workStatus->setInitStatus(_begining_plan, "");
	}

	clusterSegNode();
	//size_t length_plan = _day_segnode_map.size(); //TODO: ���ڳ�ʼ����ʱ�� //8-15
	
	size_t iter = 0;
	for (/*size_t iter = 0; iter < length_plan; iter++*/ //8-15
		std::map<time_t, SegNodeSet>::iterator it = _daytime_segnode_map.begin(); it != _daytime_segnode_map.end(); it++) {
		
		//setCurDayStartNodeSet(iter); //8-15
		_cur_day_segnode_set = &it->second;
		
		_segNet->updateResourceAndSink();
		_segNet->updateVirtualArcs(*_cur_day_segnode_set);
		// 2.crew group searching
		CrewGroup initial_group;
		initialCrewGroup(initial_group);
		std::vector<CrewGroup*> initial_groups;
		initial_groups.emplace_back(&initial_group);
		// 3.seg path searching
		/*SubProblem initial_soln(*_crewNet, *_segNet, *_rules);
		initial_soln.findSegPaths();
		initial_soln.getCurLocalPool();*/
		//initial_soln
		// 4.
		ColumnGeneration* column_generation = new ColumnGeneration();
 		column_generation->init(iter, initial_groups, *_crewNet, *_segNet, *_rules, *_penalty);
		column_generation->solve();

		Solution* cur_day_soln = new Solution(column_generation->getBestSoln());
		soln_pool.emplace_back(cur_day_soln);
		//update crew's and seg's status according cur day's decision
		updateStatus(*cur_day_soln);

		cur_day_sch_file.str("");
		cur_day_crew_status_file.str("");
		cur_day_sch_file << "../data/output/crew_schedule/schedule_day_" << std::to_string(iter + 1) << ".txt";
		cur_day_crew_status_file << "../data/output/crew_schedule/crew_status_day_" << std::to_string(iter + 1) << ".txt";
		//output_handler.writeSchedule(*cur_day_soln, *_cur_day_segnode_set, cur_day_sch_file.str());
		//output_handler.writeCrewStatus(*cur_day_soln, cur_day_crew_status_file.str());
		////��¼��������crew��״̬
		//output_handler.writeCrewStatus(_optCrewSet, CREW_STATUS_FILE);

		delete column_generation;
		++iter;
	}


	

}


void Optimizer::initialCrewGroup(CrewGroup& initGroup) {
	//random select a group according to rules.rankconbination
	auto combi = _rules->rankCombinationSet.front();
	std::vector<Opt_CREW*> crew_combi;
		
	/*for (const auto& crew1 : _optCrewSet) {
		for (const auto& crew2 : _optCrewSet) {
			if (crew1 != crew2) {
				crew_combi.emplace_back(crew1);
				crew_combi.emplace_back(crew2);
				if (_rules->isFasibleCombination(crew_combi)) {
					initGroup.getCrewGroup() = crew_combi;		
					return;
				}
			}			
		}		
	}*/
	for (size_t i = 2; i < _crewNet->nodeSet.size(); i++) {
		auto crew_node1 = _crewNet->nodeSet[i];
		for (size_t j = i+1; j < _crewNet->nodeSet.size(); j++) {
			auto crew_node2 = _crewNet->nodeSet[j];
			
			crew_combi.emplace_back(crew_node1->optCrew);
			crew_combi.emplace_back(crew_node2->optCrew);
			if (_rules->isFasibleCombination(crew_combi)) {
				initGroup.getNodeSequence().emplace_back(crew_node1);
				initGroup.getNodeSequence().emplace_back(crew_node2);

				initGroup.setCrewGroup();
				initGroup.setCrewIndexSet();
				initGroup.setBasicProperties();
				return;
			}
			
		}
	}

}

void Optimizer::clusterSegNode() {	
	for (const auto& segnode : _segNet->nodeSet) {
		_day_segnode_map[segnode->optSegment->getDate()].emplace_back(segnode);
	
		_daytime_segnode_map[getStartTimeOfDay(segnode->optSegment->getStartTimeLocSch())].emplace_back(segnode);
	
	}
	
}
void Optimizer::setCurDayStartNodeSet(int curDay) {
	time_t cur_day = _begining_plan + curDay * _SECONDS_ONE_DAY;
	while (_daytime_segnode_map.find(cur_day) == _daytime_segnode_map.end()) {
		cur_day += _SECONDS_ONE_DAY;
	}
	_cur_day_segnode_set = &_daytime_segnode_map[cur_day];
}

//! ��ǰ�׶ε�duty��departureDt��crew�ڵ�ǰ�׶ο�ʼʱ��endDtLoc֮���ʱ��������Ҫ�жϸü���Ƿ�������day off
//! ����ʱ��������day off����ôʵ���ϣ�crew���ǽ�����һ��day off
void Optimizer::updateStatus(Solution& soln) {
	ColumnPool& pool = soln.column_pool;
	Column* col;
	for (size_t i = 0; i < pool.size(); i++) {
		col = pool[i];
		auto segpath = col->_segpath;
		
		auto segnode_set = segpath->getNodeSequence();
		for (auto& segnode : segnode_set) {
			segnode->optSegment->setAssigned(true);
		}

		auto crew_set = col->_crewgroup->getCrewGroup();
		for (auto& crew : crew_set) {
			crew->workStatus->setAssigned(true);
			//TODO: work time tobe updated
			if (col->type == ColumnType::relax) {
				crew->workStatus->endDtLoc += 86400;
			}
			else {
				crew->workStatus->endDtLoc = segpath->endNode->endDtLoc;
				crew->workStatus->restStation = segpath->endNode->arvStation;
			}
			
			crew->workStatus->accumuFlyMin += segpath->total_fly_mint;
			crew->workStatus->accumuCreditMin += segpath->total_credit_mint;
			crew->workStatus->totalFlyMint += segpath->total_fly_mint;
			crew->workStatus->totalCreditMint += segpath->total_credit_mint;
		}

	}
}



/****************/
void Optimizer::loadData(std::map<std::string, std::vector<void*>>& dataSet, const std::vector<std::string>& objNameSet) {
	_inputHandler.inputData(dataSet, objNameSet);
}

void Optimizer::loadCrewRules(CrewRules& rules) {
	_rules = &rules;
}
void Optimizer::loadPenaltySetting(const Penalty& penaltySeeting) {
	_penalty = &penaltySeeting;
}
void Optimizer::init() {
	_inputHandler.sortCrewRank();
	
	_inputHandler.matchOptSegmentSet(&_optSegSet);
	_inputHandler.matchOptSegmentAndComposition(&_optSegSet);	
	_inputHandler.matchOptCrewSet(&_optCrewSet);	
	_inputHandler.matchOptCrewAndRank(&_optCrewSet);
	_inputHandler.matchOptCrewAndBase(&_optCrewSet);	
	
	
	for (auto& crew : _optCrewSet) {
		crew->setCurRank();
		crew->setCurPosition();
	}
	_crewNet = new CrewNetwork(&_optCrewSet, _rules);
	_segNet = new SegNetwork(&_optSegSet, &_inputHandler.getBaseSet(), _rules);
	
}




void Optimizer::createCase() {
	_inputHandler.setAirportSet();
	_inputHandler.createSpecialArpSet();
	
	_inputHandler.randomSetCrewSkills(&_optCrewSet);
	_inputHandler.setRankCombination(_rules);

	_optCrewSet = *_inputHandler.getPilotSet(_optCrewSet);

	_inputHandler.setIndexOfCrew(&_optCrewSet);
}