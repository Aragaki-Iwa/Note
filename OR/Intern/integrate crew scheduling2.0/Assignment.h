#pragma once
#include "pch.h"
#include "..\csvReader\crewDB_mine.h"
#include "FindDutySet.h"

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

using seqIdVec = std::vector<int>; //���seq��_rankCombinationSet�е�id

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
	//����������ֻ��Ҫ��ʼ��һ�εı���
	void init(std::vector<CREW*>* p_crewSet, 
		CrewRules* p_rules, 
		std::map<std::string, posOrderSeqVec*>* p_pos_order_seqs,
		std::map<int, orderPosSeqVec*>* p_order_pos_seqs,
		std::vector<CREW*>* p_curCAPCrewSet,
		std::vector<CREW*>* p_curFOCrewSet,
		std::vector<CREW*>* p_curAttendantCrewSet);
	
	//�õ�set covering������õ�dutySet
	//��������assign����ֻ��ִ��һ��
	void receiveSetCoverInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet);
	void labelDecidedDuty(std::vector<string>& specialAirport);		

	

	/*----------output-process----------*/
	std::vector<Path*>* getAssignedDutySet();
	
	//����+�ͷ�
	void clear();

private:		
	//��init֮�����
	void initRankCompositionMap();

	//���㻹δ������ǰ�ĳ�ʼ�ܷ�ʱ
	//��������assign����ֻ��ִ��һ��
	void initFlyMints();
	//��������assign����ֻ��ִ��һ��
	//ִ����labelDecidedDuty��ִ�иú���
	void clusterDutyByDay();
	


	//ΪcrewѰ�ң����죩�ܵ�����duty
	//����ִ��
	void initCrewDutyColumn(/*std::vector<int>& decidedDutysID*/std::vector<Path*>& decidedDutys);
	////initCrewDutyColumn�󣬸���crew.dutyColumn��ֵ
	//void setCrewDutyMatchAdj();

	//ÿ����һ������ָ�ɣ��Ϳ��Զ�crewSet�����ڲ����������ʱ����ܻ�Ƶ������
	void sortCrewSet();
	
	

	/*----------algorithm-process----------*/
	//void initialSolution(); //based duty	
	//�������duty
	void initialSolution_baseCrewAndDay();
	
	//�ж�comb�е�crew�Ƿ��������Լ��
	//���Ƚ�comb����CAP|FO��˳������
	//Ȼ���ж�
	bool isFasibleCombination(const std::vector<CREW*>& crewComb);
	void updateDutyRankNums(Path* duty, const string& rank);

	bool isCurSegCoverFinished();
	
	void calObjValue(Solution& solution);
	//���ݶ�Ӧ��solution������_CrewFlyMints��Ϊ�м���������objvalueʱ����
	void updateCrewFlyMints(Solution& solution);
	

	
	//����ָ���㷨����ֹ
	bool terminate();		

	/*----------post-process----------*/
	//���̶ĵķ�ʽ
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

	
	int _initial_sumFlyMint; //������Ҫday off��crew�ķ�ʱ	
	std::map<int, std::map<string, string>> _requireNum_pos_seq;

	//std::vector<int> _CrewFlyMints; //_crweSet��crew������day off��crew����flyMint����Ӧ��ÿ������仯  
	std::map<CREW*, int> _CrewFlyMintsMap;
	
	//std::map<string, std::vector<int>> _decidedDutyInDaysID; //�洢����duty��_decidedDutySet�е�index
	std::map<string, std::vector<Path*>> _decidedDutyInDays;
	
	//std::vector<std::vector<int>> _dutyCrewAdj; //index d,c are duty index in _decidedDutySet and crew index in _curCrewSet
	//std::vector<std::vector<int>> _crewDutyAdj; //��_dutyCrewAdj��Ϊת��
	
	std::map<CREW*, std::vector<Path*>> _crewDutyPairs;
	std::map<Path*, std::vector<CREW*>> _dutyCrewPairs;

	Solution* _initialSoln;
	std::vector<Solution*> _solnPool;
	Solution* _optSoln;
};
#endif // !ASSIGNMENT_H

