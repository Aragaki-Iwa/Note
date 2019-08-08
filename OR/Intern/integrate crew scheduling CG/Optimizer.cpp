#include "Optimizer.h"
#include "..\csvReader\UtilFunc.h"
#include "csvClassesTransOptimizable.h"
#include "CrewRules.h"
#include "Seg_Network.h"
#include "Crew_Network.h"
#include "Crew_Path.h"
#include "Seg_Path.h"
#include "CrewGroupSearcher.h"

//! for debug
#include "SummeryTool_H.h"
static Summery::StopWatch TIMER;
//! end for debug

Optimizer::Optimizer() {	
	_column_generation = new ColumnGeneration();
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

	
	_begining_plan = getStartTimeOfDay(_segNet->nodeSet.front()->startDtLoc);
	for (auto& crew : _optCrewSet) {
		crew->workStatus->setInitStatus(_begining_plan, "");
	}

	clusterSegNode();
	size_t length_plan = _day_segnode_map.size(); //TODO: 放在初始化的时候

	for (size_t iter = 0; iter < length_plan; iter++) {
		
		setCurDayStartNodeSet(iter);
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
		_column_generation->init(initial_groups, *_crewNet, *_segNet, *_rules, *_penalty);
		_column_generation->solve();
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
	_cur_day_segnode_set = &_daytime_segnode_map[cur_day];
}

void Optimizer::updateStatus(Solution& soln) {
	auto pool = soln.column_pool;
	for (size_t i = 0; i < pool->size(); i++) {
		auto segnode_set = (*pool)[i]->_segpath->getNodeSequence();
		for (auto& segnode : segnode_set) {
			segnode->optSegment->setAssigned(true);
		}
		auto crew_set = (*pool)[i]->_crewgroup->getCrewGroup();
		for (auto& crew : crew_set) {
			crew->workStatus->setAssigned(true);
			//TODO: work time tobe updated

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