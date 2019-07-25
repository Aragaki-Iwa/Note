#pragma once
#include "pch.h"
#include "..\csvReader\crewDB_mine.h"
#include "FindDutySet.h"

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

using seqIdVec = std::vector<int>; //存放seq在_rankCombinationSet中的id

static vector<string> CAP_positions{ "C1", "C4", "T1", "T2", "T3", "K1", "K2" };
static vector<string> FO_positions{ "E1", "F1", "F2", "F3", "F4", "F5", "F6", "J1", "J2" };
struct posOrderSeqVec
{	
	std::string position;	
	std::map<int, seqIdVec> orderSeqIdvec;
};
struct orderPosSeqVec
{
	int order;
	std::map<std::string, seqIdVec> posSeqIdvec;
};


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
	Solution() {
		CrewDutyPairs.clear();
		DutyCrewPairs.clear();
	}
	~Solution() {
		CrewDutyPairs.clear();
		DutyCrewPairs.clear();
	}
	//Solution(const int numRow, const int numColumn);	
	/*std::vector<std::vector<int>> CrewDutyMatrix;
	std::vector<std::vector<int>> CrewDutyAdj;
	std::vector<std::vector<int>> DutyCrewAdj;*/

	std::map<CREW*, std::vector<Path*>> CrewDutyPairs;
	std::map<Path*, std::vector<CREW*>> DutyCrewPairs;
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
	void init(std::vector<CREW*>* p_crewSet, 
		CrewRules* p_rules, 
		std::map<std::string, posOrderSeqVec*>* p_pos_order_seqs,
		std::map<int, orderPosSeqVec*>* p_order_pos_seqs,
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
	//在init之后调用
	void initRankCompositionMap();

	//计算还未做决策前的初始总飞时
	//整个单次assign过程只需执行一次
	void initFlyMints();
	//整个单次assign过程只需执行一次
	//执行完labelDecidedDuty后执行该函数
	void clusterDutyByDay();
	


	//为crew寻找（当天）能担当的duty
	//按天执行
	void initCrewDutyColumn(/*std::vector<int>& decidedDutysID*/std::vector<Path*>& decidedDutys);
	////initCrewDutyColumn后，根据crew.dutyColumn赋值
	//void setCrewDutyMatchAdj();

	//每进行一次完整指派，就可以对crewSet排序。在不断搜索解的时候可能会频繁调用
	void sortCrewSet();
	
	

	/*----------algorithm-process----------*/
	//void initialSolution(); //based duty	
	//按天分配duty
	void initialSolution_baseCrewAndDay();
	
	//判断comb中的crew是否满足搭配约束
	//首先将comb按：CAP|FO的顺序排列
	//然后判断
	bool isFasibleCombination(const std::vector<CREW*>& crewComb);
	void updateDutyRankNums(Path* duty, const string& rank);

	bool isCurSegCoverFinished();
	
	void calObjValue(Solution& solution);
	//根据对应的solution，更新_CrewFlyMints。为中间量，计算objvalue时调用
	void updateCrewFlyMints(Solution& solution);
	

	
	//整个指派算法的终止
	bool terminate();		

	/*----------post-process----------*/
	//轮盘赌的方式
	void selectDayoffCrew(/*std::vector<CREW*>& crewSet*/);

	void postProcess();
	
	void updateCrewStatus(CREW* crew, Path* duty);

	//debug
	string _uncoveredFltFile = "debug-uncoverFlt.csv";
	ofstream _outStream;
	flightParser _fltParser;

	//input
	CrewRules* _rules;
	std::map<std::string, posOrderSeqVec*>* _pos_order_seqs_ptr;
	std::map<int, orderPosSeqVec*>* _order_pos_seqs_ptr;

	std::vector<CREW*>* _crewSet;
	std::vector<CREW*>* _curCAPCrewSet;
	std::vector<CREW*>* _curFOCrewSet;
	std::vector<CREW*>* _curAttendantCrewSet;

	std::vector<Node*>* _curSegNodeSet;
	std::vector<Path*>* _decidedDutySet;
	//output
	std::vector<Path*>* _assignedDutySet;

	
	int _initial_sumFlyMint; //包括需要day off的crew的飞时	
	std::map<int, std::map<string, string>> _requireNum_pos_seq;

	//std::vector<int> _CrewFlyMints; //_crweSet中crew（包括day off的crew）的flyMint。对应于每个解而变化  
	std::map<CREW*, int> _CrewFlyMintsMap;
	
	//std::map<string, std::vector<int>> _decidedDutyInDaysID; //存储的是duty在_decidedDutySet中的index
	std::map<string, std::vector<Path*>> _decidedDutyInDays;
	
	//std::vector<std::vector<int>> _dutyCrewAdj; //index d,c are duty index in _decidedDutySet and crew index in _curCrewSet
	//std::vector<std::vector<int>> _crewDutyAdj; //和_dutyCrewAdj互为转置
	
	std::map<CREW*, std::vector<Path*>> _crewDutyPairs;
	std::map<Path*, std::vector<CREW*>> _dutyCrewPairs;

	Solution* _initialSoln;
	std::vector<Solution*> _solnPool;
	Solution* _optSoln;
};
#endif // !ASSIGNMENT_H

