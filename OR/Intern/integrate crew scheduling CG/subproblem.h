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
	//！ 整个单天决策（CG过程中）只进行一次
	void findSegPaths();
	void setPathStatus(const Penalty& penalty);
	//! 先调用了私有函数得到当天的所有segPath
	void setCurDaySegSet();
	void setIndexOfCurSeg();
	void labelSpecialSegPath(std::vector<std::string>& specialAirports);
	//! 对segpath按配比分类
	void groupingSegPathByComposition();
	
	
	/*估值的功能不应该由subproblem实现，均值应该作为subproblem的一个参数*/	
	//! 根据segpath_set和crew_set对flymint的均值进行预估
	//! 一天执行一次即可
	void estimateMeanFlyMin();
	//! 不同rank有各自的估计均值
	//! 计算公式：mean_fly_min_rank[i] = sum(seg.fly_min * required_num_rank[i]) / total_num_rank[i]
	void estimateMeanFlyMinPrecisly();
	
	int getMeanFlyMin() const { return _mean_fly_mint; }

	void updateDuals(std::vector<double>& segCoverDuals, std::vector<double>& crewAssignDuals);
	void findGroups();
	
	
	//! create new columns, add to this->_local_pool
	//! match crew group and segment path
	//! to get columns that have small negetive reduced cost
	//! actually, is assign group for each segpath in _seg_path_searcher._segpath_set
	void matchGroupAndPath();	

	//! 将搜索crewGroup和匹配Group与segpath合并
	void findColumnSet();
	
	//! 要赋值，segpath的时间属性
	void addRestColumns();

	/*output for master problem*/
	
	//! 只传递一次，主问题初始化的时候
	std::vector<Opt_Segment*>& getCurDaySegSet() { return _cur_day_seg_set; }

	ColumnPool& getCurLocalPool() { return _local_pool; }

private:		
	//! 根据预估的flymint均值，计算所有column的cost
	//! cost = sum(crewGroup's flyMin + dutyFlyMin - meanFlyMin)	
	int calBalanceCost(const int dutyFlyMin, CrewGroup& crewGroup);
	//! 根据预估的各rank的flymint均值，计算所有column的均衡cost
	int calBalanceCostByVariance(const int dutyFlyMin, CrewGroup& crewGroup);
	
	//! 按opt_crew的对偶价格排序
	void sort_pos_crew_set();
	
	//! 根据compoMode搜索合法的crewGroup		
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
	std::vector<CrewNode*> _crewnode_set; //TODO:未初始化
	ColumnPool _local_pool; //每次子问题得到的新列

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

