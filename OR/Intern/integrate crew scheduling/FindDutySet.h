#pragma once
#include "pch.h"
#include "Network.h"
#include "..\csvReader\crewDB_mine.h"

class Path
{
public:
	std::vector<Node*> route;
	//int* coverArray;
	std::vector<int> coverVec;
	int flyMin;
	int workMin;
	time_t startDtLoc;
	time_t endDtLoc;
	string startStation;
	string endStation;

	string startDate;

	std::map<string, int> specialCredentials;
	//担当该duty的crew
	std::vector<CREW*> work_crewSet;

	//for basedCrew
	std::list<int> crewID;//两个乘务员:CAP和FO，由互斥矩阵限制，不存在两个同级别的不可能出现在同一duty上

};


class PathFinder
{
public:
	std::vector<Path*>* getPathSet();
	void findPathSet(Network* net, vector<Node*>& startNodeSet, const CrewRules& rules);

private:
	void initResourceNode(Node* startNode);
	//建立当天（也不一定是当天，以前一阶段的duty结束点汇集为一个源节点）的源节点
	//总之，找路的第一个点，即startNode一定是一个虚拟点
	Node* createCurResource(std::vector<Node*>& startNodeSet);
	
	void dfs(Node* startNode, const CrewRules& rules);
	void dfs_nonrecursive(Node* startNode, const CrewRules& rules);
	void dfs_nonrecursive2(Node* startNode, const CrewRules& rules);
	std::map<Arc*, int> _VisitedByPath;

	void extend(const Label* label, Arc* curArc, Label* nextLabel);
	bool checkFeasible(const Label& label, Arc* curArc, Label* nextLabel, const CrewRules& crewRules);
	bool checkTermination(const Label& curLabel, const Label& nextLabel, const CrewRules& crewRules);		
	//路就是duty，是基本单位，所以不需要在找路的时候判断dayoff，而是在crew每次得到duty分配方案后在判断，不属于findPath职能
	//bool checkDayOff(const Label& label, Label* nextLabel, const CrewRules& crewRules);

	bool checkDeadhead(Label* nextLabel);
	Node* findDeadhead(Label* nextLabel);
	void updateLabel(Label* label, Arc* arc, Label* nextLabel);
	Path* backtrack(Label* endLabel);

	std::list<Arc*> _arcList;
	std::vector<Path*> _pathSet; //each path in the set will be delete after one iteration

};
