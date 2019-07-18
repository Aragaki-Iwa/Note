#pragma once
#include "pch.h"
#include "..\csvReader\crewDB_mine.h"
#include "FindDutySet.h"

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

struct Objecive
{
	double objValue;
	
	double flyMintVariance;
	double fltCancelCost;
	double dhdCost;
};
class Solution
{
public:
	Solution() {}
	Solution(const int numRow, const int numColumn);
	//int** CrewDutyMatrix;	
	std::vector<std::vector<int>> CrewDutyMatrix;
	std::vector<std::vector<int>> CrewDutyAdj;
	std::vector<std::vector<int>> DutyCrewAdj;
	Objecive obj;

};

class Assigner
{
public:
	//return solve status
	//different value means different status
	int solve();

	/*----------pre-process----------*/	
	//只需要初始化一次的变量
	void init(std::vector<CREW*>* p_crewSet, CrewRules* p_rules,
		std::vector<CREW*>* p_curCAPCrewSet,
		std::vector<CREW*>* p_curFOCrewSet,
		std::vector<CREW*>* p_curAttendantCrewSet);
	//得到set covering求解所得的dutySet
	void receiveInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet);
	void labelDecidedDuty(std::vector<string>& specialAirport);		

	

	/*----------output-process----------*/
	std::vector<Path*>* getAssignedDutySet();
	
	//重置+释放
	void clear();

private:		
	void clusterDutyByDay();
	//检查crew的各种累计工作时间，分组
	void initCrewDutyColumn(std::vector<Path*>& decidedDutys);
	void sortCrewSet();
	//计算还未做决策前的初始总飞时
	void initFlyMint();
	void initMatrixs();


	/*----------algorithm-process----------*/
	void initialSolution(); //based duty
	int initialSolution_basedCrew();
	//按天分配duty
	void initialSolution_baseCrewAndDay();
	
	
	bool isRankMatch(CREW_RANK* cap, CREW_RANK* fo);
	//单次解的搜索完成
	bool stop();
	
	void calObjValue(Solution& solution);
	//根据对应的solution，更新_CrewFlyMints。为中间量，计算objvalue时调用
	void updateCrewFlyMints(Solution& solution);
	

	
	//整个指派算法的终止
	bool terminate();

	void updateMatrix();

	/*----------post-process----------*/
	void postProcess();
	
	void updateCrewStatus();

	//debug
	string _uncoveredFltFile = "debug-uncoverFlt.csv";
	ofstream _outStream;
	flightParser _fltParser;

	//input
	CrewRules* _rules;
	std::vector<CREW*>* _crewSet;
	std::vector<Node*>* _curSegNodeSet;
	std::vector<Path*>* _decidedDutySet;
	//output
	std::vector<Path*>* _assignedDutySet;

	
	int _initial_sumFlyMint; //包括需要day off的crew的飞时
	/*std::vector<CREW*> _curDayOffCrewSet;
	std::vector<CREW*> _curCrewSet;	*/
	std::vector<int> _CrewFlyMints; //_crweSet中crew（包括day off的crew）的flyMint。对应于每个解而变化  

	std::vector<CREW*>* _curCAPCrewSet;
	std::vector<CREW*>* _curFOCrewSet;
	std::vector<CREW*>* _curAttendantCrewSet;

	//std::vector<std::vector<Path*>> _decidedDutyInDays;
	std::map<string, std::vector<Path*>> _decidedDutyInDays;

	std::vector<std::vector<int>> _crewMutualMatrix; //index i,j is crew index in _curCrewSet 
	std::vector<std::vector<int>> _dutyCrewAdj; //index d,c are duty index in _decidedDutySet and crew index in _curCrewSet
	std::vector<std::vector<int>> _crewDutyAdj; //和_dutyCrewAdj互为转置

	Solution* _initialSoln;
	std::vector<Solution*> _solnPool;
	Solution* _optSoln;
};
#endif // !ASSIGNMENT_H

