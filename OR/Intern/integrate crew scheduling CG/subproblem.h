#pragma once
#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H
#include "pch.h"
#include "CrewGroupSearcher.h"
#include "segpath_searcher.h"
#include "Crew_Network.h"
#include "Seg_Network.h"
#include "csvClassesTransOptimizable.h"
#include "CrewRules.h"
#include "column.h"
#include "cost_parameters.h"


class SubProblem 
{
public:
	SubProblem(CrewNetwork& crewNet, SegNetwork& segNet, CrewRules& rules);
	~SubProblem();
	
	void setInitialGroups(std::vector<CrewGroup*>& initialGroups);
	//�� ����������ߣ�CG�����У�ֻ����һ��
	void findSegPaths();
	void setPathStatus(const Penalty& penalty);
	//! �ȵ�����˽�к����õ����������segPath
	void setCurDaySegSet();
	void setIndexOfCurSeg();
	void labelSpecialSegPath(std::vector<std::string>& specialAirports);
	//! ��segpath����ȷ���
	void groupingSegPathByComposition();
	
	
	/*��ֵ�Ĺ��ܲ�Ӧ����subproblemʵ�֣���ֵӦ����Ϊsubproblem��һ������*/	
	//! ����segpath_set��crew_set��flymint�ľ�ֵ����Ԥ��
	//! һ��ִ��һ�μ���
	void estimateMeanFlyMin();
	//! ��ͬrank�и��ԵĹ��ƾ�ֵ
	//! ���㹫ʽ��mean_fly_min_rank[i] = sum(seg.fly_min * required_num_rank[i]) / total_num_rank[i]
	void estimateMeanFlyMinPrecisly();
	
	int getMeanFlyMin() const { return _mean_fly_mint; }

	void updateDuals(std::vector<double>& segCoverDuals, std::vector<double>& crewAssignDuals);
	void findGroups();
	
	
	//! create new columns, add to this->_local_pool
	//! match crew group and segment path
	//! to get columns that have small negetive reduced cost
	//! actually, is assign group for each segpath in _seg_path_searcher._segpath_set
	void matchGroupAndPath();	

	//! ������crewGroup��ƥ��Group��segpath�ϲ�
	void findColumnSet();
	
	//! Ҫ��ֵ��segpath��ʱ������
	void addRestColumns();

	/*output for master problem*/
	
	//! ֻ����һ�Σ��������ʼ����ʱ��
	std::vector<Opt_Segment*>& getCurDaySegSet() { return _cur_day_seg_set; }

	ColumnPool& getCurLocalPool() { return _local_pool; }

private:		
	//! ����Ԥ����flymint��ֵ����������column��cost
	//! cost = sum(crewGroup's flyMin + dutyFlyMin - meanFlyMin)	
	int calBalanceCost(const int dutyFlyMin, CrewGroup& crewGroup);
	//! ����Ԥ���ĸ�rank��flymint��ֵ����������column�ľ���cost
	int calBalanceCostByVariance(const int dutyFlyMin, CrewGroup& crewGroup);
	
	//! ��opt_crew�Ķ�ż�۸�����
	void sort_pos_crew_set();
	
	//! ����compoMode�����Ϸ���crewGroup		
	void searchGroupByComposition(const std::string compositionName, std::vector<CrewGroup*>* crewGroupSet);
	
	bool isMatchable(CrewGroup& group, SegPath& segPath);

	std::map<std::string, std::vector<CrewNode*>> _pos_crewnodes_map;
	std::map<std::string, int> _rank_to_num_crew_map;

	/*struct partialGroup
	{
		std::vector<CrewNode*> crewnode_seq;
		double sum_price;
	};*/


	CrewRules* _rules;
	std::vector<Opt_Segment*> _cur_day_seg_set;
	std::vector<CrewNode*> _crewnode_set; //TODO:δ��ʼ��
	ColumnPool _local_pool; //ÿ��������õ�������

	GroupSearcher _group_searcher;
	SegPathSearcher _seg_path_searcher;
	std::vector<SegPath*> _cur_day_path_set;
	
	int _mean_fly_mint;
	std::map<std::string, int> _rank_to_mean_fly_mint;
	/*std::vector<double> _seg_cover_duals;
	std::vector<double> _crew_assign_duals;
	std::vector<double> _balance_flytime_duals;
	std::vector<double> _balance_resttime_duals;*/

	std::map<std::string, std::vector<SegPath*>> _compo_mode_segpath_set;


};

#endif // !SUBPROBLEM_H

