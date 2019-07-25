#pragma once
#include "InputHandle.h"
#include "FindDutySet.h"
#include "CplexModel.h"
#include "Assignment.h"
#ifndef INTEGRATE_OPTIMIZATION_H
#define INTEGRATE_OPTIMIZATION_H

template<class T>
void permutations(std::vector<T>& data, int begin, int end, std::vector<T>* permSet);

class RollingOpt
{
public:
	RollingOpt() {
		_net = new Network();
	}
	~RollingOpt() {
		delete _net;
	}

	std::vector<CREW*> getCrewSet();
	void optimize();	
	void inputData(std::map<string, std::vector<void*>>& dataSet, const std::vector<string>& objNameSet);
	void setRules(CrewRules& rules);
	//��ʼ���������ݣ���crew��crewrank������Ե�
	void init();
	//�������������ѡ��specialAirport��specialCrew
	void randomSet_specialAirport();
	void randomSet_crewSkills(double percent = 0.3);
	void set_rankCombination();	
	void setSeqMaps();
	void sort_SeqMaps();

private:
	//�������ֻ��Ҫһ��
	void clusterCrewMeb();
	
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
	std::vector<csvActivityComposition*> _fltCompositionSet;
	std::vector<csvComposition*> _compositionSet;

	std::vector<std::string> _special_airportSet;
	std::vector<string> _rankCombinationSet;
	std::map<std::string, posOrderSeqVec*> _pos_order_seqs;
	std::map<int, orderPosSeqVec*> _order_pos_seqs;

	std::vector<CREW*> _curCAPCrewSet;
	std::vector<CREW*> _curFOCrewSet;
	std::vector<CREW*> _curAttendantCrewSet;
	
	Network* _net;
	CrewRules* _rules;
	
	
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
