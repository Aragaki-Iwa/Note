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
	//��ʼ���������ݣ���crew��crewrank������Ե�
	void init();
	//�������������ѡ��specialAirport��specialCrew
	void randomSet_specialAirport();
	void randomSet_crewSkills(double percent = 0.3);
	void set_rankCombination();
	
	

private:
	void clusterCrewMeb();
	//�������ֻ��Ҫһ��
	void initMutrualMatrix();
	bool isRankMatch(CREW_RANK* cap, CREW_RANK* fo);
	void initialStartNodeSet();
	void removeDhdDuty(std::vector<Path*>& dutySet);
	void updateStartNodeSet(int iterDay);
	//��û��segment������Ϊ���ʱ�������е�segment���Ǳ�crew���������Ǳ�ȡ��������
	//���һ���ʱ����Ҫע�⴦��
	//����ɨ������network��ֻ��ӵ�ǰ�����˾��ߣ�����䣬��ȡ������duty�����У�����ʱ�������segment�����������֮���segment��
	//��������������ȽϺ�
	//�������ˡ����������Ӻ���ǰɨ��net������
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
	//ÿ����·ǰҪ��������㣬�����ϴ�cplex�Ľ�
	std::vector<Node*> _cur_start_nodeSet;
	//��ǰҪ�����ߵ�duty����
	std::vector<Path*> _cur_dutySet;
	//����ǰ�׶������Ѿ����߹���duty�ļ���
	std::vector<Path*> _decided_dutySet;
};

#endif // !INTEGRATE_OPTIMIZATION_H
