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
	//整个求解过程只需要初始化一次的变量
	void init(std::vector<CREW*>* p_crewSet, CrewRules* p_rules, std::vector<std::vector<int>>* crewMutualMatrix,
		std::vector<CREW*>* p_curCAPCrewSet,
		std::vector<CREW*>* p_curFOCrewSet,
		std::vector<CREW*>* p_curAttendantCrewSet);
	
	//得到set covering求解所得的dutySet
	//整个单次assign过程只需执行一次
	void receiveSetCoverInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet);
	void labelDecidedDuty(std::vector<string>& specialAirport);		

	

	/*----------output-process----------*/
	std::vector<Path*>* getAssignedDutySet();
	
	//重置+释放
	void clear();

private:		
	//计算还未做决策前的初始总飞时
	//整个单次assign过程只需执行一次
	void initFlyMints();
	//整个单次assign过程只需执行一次
	//执行完labelDecidedDuty后执行该函数
	void clusterDutyByDay();
	


	//为crew寻找（当天）能担当的duty
	//按天执行
	void initCrewDutyColumn(std::vector<int>& decidedDutysID);	
	//initCrewDutyColumn后，根据crew.dutyColumn赋值
	void setCrewDutyMatchAdj();

	//每进行一次完整指派，就可以对crewSet排序。在不断搜索解的时候可能会频繁调用
	void sortCrewSet();
	
	

	/*----------algorithm-process----------*/
	void initialSolution(); //based duty
	int initialSolution_basedCrew();
	//按天分配duty
	void initialSolution_baseCrewAndDay();
	
	
		
	bool isCurSegCoverFinished();
	
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
	std::vector<std::vector<int>> _crewMutualMatrix; //index i,j is crew index in _curCrewSet 
	std::vector<CREW*>* _crewSet;
	std::vector<CREW*>* _curCAPCrewSet;
	std::vector<CREW*>* _curFOCrewSet;
	std::vector<CREW*>* _curAttendantCrewSet;

	std::vector<Node*>* _curSegNodeSet;
	std::vector<Path*>* _decidedDutySet;
	//output
	std::vector<Path*>* _assignedDutySet;

	
	int _initial_sumFlyMint; //包括需要day off的crew的飞时
	/*std::vector<CREW*> _curDayOffCrewSet;
	std::vector<CREW*> _curCrewSet;	*/
	
	std::vector<int> _CrewFlyMints; //_crweSet中crew（包括day off的crew）的flyMint。对应于每个解而变化  


	//std::vector<std::vector<Path*>> _decidedDutyInDays;
	std::map<string, std::vector<int>> _decidedDutyInDaysID; //存储的是duty在_decidedDutySet中的index

	
	std::vector<std::vector<int>> _dutyCrewAdj; //index d,c are duty index in _decidedDutySet and crew index in _curCrewSet
	std::vector<std::vector<int>> _crewDutyAdj; //和_dutyCrewAdj互为转置

	Solution* _initialSoln;
	std::vector<Solution*> _solnPool;
	Solution* _optSoln;
};
#endif // !ASSIGNMENT_H

