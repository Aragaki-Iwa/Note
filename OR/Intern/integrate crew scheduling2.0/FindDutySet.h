#pragma once
#include "pch.h"
#include "Network.h"
#include "..\csvReader\crewDB_mine.h"

struct compoMode
{			
	std::string name;
	int totalNum;
	std::map<std::string, int> rankNum;
	
};


static std::vector<compoMode> compoModes = { {"1CAP1FO", 2, {{"CAP",1}, {"FO",1}} },
	{"1CAP2FO", 3, {{"CAP",1}, {"FO",2}} },
	{"2CAP1FO", 3, {{"CAP",2}, {"FO",1}} },
	{"3CAP1FO", 4, {{"CAP",3}, {"FO",1}} } };

class Path
{
public:
	void init();
	void resetCompoNums();
	std::vector<Node*> route;	
	string startDate;
	int flyMin;
	int workMin;
	time_t startDtLoc;
	time_t endDtLoc;
	string startStation;
	string endStation;

	std::vector<int> coverVec;

	std::map<string, int> specialCredentials;
	//������duty��crew
	std::vector<CREW*> work_crewSet;

	//for basedCrew	
	std::vector<CREW*> crewCombination;//������duty��crew����
		
	//�������б仯
	struct compoMode tempCompoMode;
	//std::map<std::string, int> tempRankNums;
	bool assigned;

private:
	//һ����ʼ���㲻��ı�
	const csvComposition* composition;
	//һ����ʼ���㲻��ı�
	const compoMode* compoMode;

	
};


class PathFinder
{
public:
	std::vector<Path*>* getPathSet();
	void findPathSet(Network* net, vector<Node*>& startNodeSet, const CrewRules& rules);

private:
	void initResourceNode(Node* startNode);
	//�������죨Ҳ��һ���ǵ��죬��ǰһ�׶ε�duty������㼯Ϊһ��Դ�ڵ㣩��Դ�ڵ�
	//��֮����·�ĵ�һ���㣬��startNodeһ����һ�������
	Node* createCurResource(std::vector<Node*>& startNodeSet);
	
	void dfs(Node* startNode, const CrewRules& rules);
	void dfs_nonrecursive(Node* startNode, const CrewRules& rules);
	void dfs_nonrecursive2(Node* startNode, const CrewRules& rules);
	std::map<Arc*, int> _VisitedByPath;

	void extend(const Label* label, Arc* curArc, Label* nextLabel);
	bool checkFeasible(const Label& label, Arc* curArc, Label* nextLabel, const CrewRules& crewRules);
	bool checkTermination(const Label& curLabel, const Label& nextLabel, const CrewRules& crewRules);		
	//·����duty���ǻ�����λ�����Բ���Ҫ����·��ʱ���ж�dayoff��������crewÿ�εõ�duty���䷽�������жϣ�������findPathְ��
	//bool checkDayOff(const Label& label, Label* nextLabel, const CrewRules& crewRules);

	bool checkDeadhead(Label* nextLabel);
	Node* findDeadhead(Label* nextLabel);
	void updateLabel(Label* label, Arc* arc, Label* nextLabel);
	Path* backtrack(Label* endLabel);

	std::list<Arc*> _arcList;
	std::vector<Path*> _pathSet; //each path in the set will be delete after one iteration

};
