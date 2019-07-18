#pragma once
#include "InputHandle.h"
#include "FindDutySet.h"
#include "CplexModel.h"
#include "Assignment.h"
#ifndef INTEGRATE_OPTIMIZATION_H
#define INTEGRATE_OPTIMIZATION_H


class RollingOpt
{
public:
	RollingOpt() {
		_net = new Network();
	}
	~RollingOpt() {
		delete _net;
	}
	void optimize();	
	void inputData(std::map<string, std::vector<void*>>& dataSet, const std::vector<string>& objNameSet);
	void setRules(CrewRules& rules);
	//初始化输入数据，对crew和crewrank进行配对等
	void init();
	//创建案例，随机选定specialAirport和specialCrew
	void randomSet_specialAirport();
	void randomSet_crewSkills(double percent = 0.3);
	void set_rankCombination();
	
	

private:
	void clusterCrewMeb();
	//整个求解只需要一次
	void initMutrualMatrix();
	bool isRankMatch(CREW_RANK* cap, CREW_RANK* fo);
	void initialStartNodeSet();
	void removeDhdDuty(std::vector<Path*>& dutySet);
	void updateStartNodeSet(int iterDay);
	//当没有segment可以作为起点时，即所有的segment或是被crew担当，或是被取消，结束
	//最后一天的时候需要注意处理
	//不必扫描整个network，只需从当前已做了决策（或分配，或取消）的duty集合中，结束时间最早的segment出发，检查它之后的segment。
	//所以先拓扑排序比较好
	//不用排了。。。。。从后往前扫描net就行了
	bool termination();


	std::vector<Segment*> _segSet;
	std::vector<BASE*> _baseSet;
	std::vector<CREW*> _crewSet;
	std::vector<CREW_RANK*> _crew_rankSet;
	std::vector<CREW_BASE*> _crew_baseSet;

	std::vector<std::string> _special_airportSet;
	std::unordered_set<string> _rankCombinationSet;

	std::vector<CREW*> _curCAPCrewSet;
	std::vector<CREW*> _curFOCrewSet;
	std::vector<CREW*> _curAttendantCrewSet;
	
	Network* _net;
	CrewRules* _rules;
	std::vector<std::vector<int>> _crewMutualMatrix; //index i,j is crew index in _curCrewSet
	
	InputHandler _inputHandler;
	PathFinder _pathFinder;
	dutyModel _dutyModel;
	Assigner _assigner;
	//每次找路前要决定的起点，基于上次cplex的解
	std::vector<Node*> _cur_start_nodeSet;
	//当前要做决策的duty集合
	std::vector<Path*> _cur_dutySet;
	//到当前阶段所有已经决策过的duty的集合
	std::vector<Path*> _decided_dutySet;
};

#endif // !INTEGRATE_OPTIMIZATION_H
