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
	//ֻ��Ҫ��ʼ��һ�εı���
	void init(std::vector<CREW*>* p_crewSet, CrewRules* p_rules,
		std::vector<CREW*>* p_curCAPCrewSet,
		std::vector<CREW*>* p_curFOCrewSet,
		std::vector<CREW*>* p_curAttendantCrewSet);
	//�õ�set covering������õ�dutySet
	void receiveInput(std::vector<Path*>* p_decidedDutySet, std::vector<Node*>* p_curSegNodeSet);
	void labelDecidedDuty(std::vector<string>& specialAirport);		

	

	/*----------output-process----------*/
	std::vector<Path*>* getAssignedDutySet();
	
	//����+�ͷ�
	void clear();

private:		
	void clusterDutyByDay();
	//���crew�ĸ����ۼƹ���ʱ�䣬����
	void initCrewDutyColumn(std::vector<Path*>& decidedDutys);
	void sortCrewSet();
	//���㻹δ������ǰ�ĳ�ʼ�ܷ�ʱ
	void initFlyMint();
	void initMatrixs();


	/*----------algorithm-process----------*/
	void initialSolution(); //based duty
	int initialSolution_basedCrew();
	//�������duty
	void initialSolution_baseCrewAndDay();
	
	
	bool isRankMatch(CREW_RANK* cap, CREW_RANK* fo);
	//���ν���������
	bool stop();
	
	void calObjValue(Solution& solution);
	//���ݶ�Ӧ��solution������_CrewFlyMints��Ϊ�м���������objvalueʱ����
	void updateCrewFlyMints(Solution& solution);
	

	
	//����ָ���㷨����ֹ
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

	
	int _initial_sumFlyMint; //������Ҫday off��crew�ķ�ʱ
	/*std::vector<CREW*> _curDayOffCrewSet;
	std::vector<CREW*> _curCrewSet;	*/
	std::vector<int> _CrewFlyMints; //_crweSet��crew������day off��crew����flyMint����Ӧ��ÿ������仯  

	std::vector<CREW*>* _curCAPCrewSet;
	std::vector<CREW*>* _curFOCrewSet;
	std::vector<CREW*>* _curAttendantCrewSet;

	//std::vector<std::vector<Path*>> _decidedDutyInDays;
	std::map<string, std::vector<Path*>> _decidedDutyInDays;

	std::vector<std::vector<int>> _crewMutualMatrix; //index i,j is crew index in _curCrewSet 
	std::vector<std::vector<int>> _dutyCrewAdj; //index d,c are duty index in _decidedDutySet and crew index in _curCrewSet
	std::vector<std::vector<int>> _crewDutyAdj; //��_dutyCrewAdj��Ϊת��

	Solution* _initialSoln;
	std::vector<Solution*> _solnPool;
	Solution* _optSoln;
};
#endif // !ASSIGNMENT_H

