#pragma once
#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include "pch.h"
#include "InputHandle.h"
#include "column_generation.h"


class CrewRules;
class PosOrderSeqvec;
class OrderPosSeqvec;

class Opt_Segment;
class Opt_CREW;

//class SegNode;
//class CrewNode;
class SegNetwork;
class CrewNetwork;
class CrewGroup;
class SegPath;
class GroupSearcher;

class Optimizer
{
public:
	Optimizer();
	~Optimizer();

	//! 1.Initialize: construct crew and segment network
	//! 2.CrewgroupSearching: search crew groups
	//! 3.SegpathSearching: search seg path, actually, enumerate all feasible path in a single day
	//! 4.MatchGroupAndPath: to get a match which has the smallest (negtive) reduced cost. actually, get multiple path(column)
	//! 5.MasterModelSolver: construct master model according to columns got in last process, and solve linear relaxatiion problem
	//! 6.DualInfomationPassing: uapate dual information of crew and seg network according the dual info got from last process
	//! 7.Repeat step 2-5 unless optimal solution found
	void optimize();

	/*******input*******/
	void loadData(std::map<std::string, std::vector<void*>>& dataSet, const std::vector<std::string>& objNameSet);
	//rule也是input,所以实际应该在输入时就处理好rules,主要时组合的排列要赋好值
	void loadCrewRules(CrewRules& rules);	
	void loadPenaltySetting(const Penalty& penaltySeeting);
	//initialize input data
	void init();
	//just a temp function, for test
	void createCase();
			
	std::vector<Solution*> soln_pool;
private:	
	void initialCrewGroup(CrewGroup& initGroup);
	
	void clusterSegNode();
	void setCurDayStartNodeSet(int curDay);
	//! update status of optSeg and optCrew
	void updateStatus(Solution& soln);

	InputHandler _inputHandler;
	CrewRules* _rules;
	const Penalty* _penalty;
	
	std::vector<Opt_Segment*> _optSegSet;
	std::vector<Opt_CREW*> _optCrewSet;

	SegNetwork* _segNet;
	CrewNetwork* _crewNet;	

	/*for calculate*/
	std::map<std::string, SegNodeSet> _day_segnode_map;
	std::map<time_t, SegNodeSet> _daytime_segnode_map;
	
	time_t _begining_plan;
	const int _SECONDS_ONE_DAY = 24 * 3600; //24 hour * 3600 secends
	SegNodeSet* _cur_day_segnode_set;

	//ColumnGeneration* _column_generation;
	


};

#endif // !OPTIMIZER_H