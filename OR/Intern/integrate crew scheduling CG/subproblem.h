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

	void updateDuals(std::vector<double>& segCoverDuals, std::vector<double>& crewAssignDuals);
	void findGroups();
	int* getFlyMintRange();
	//! create new columns, add to this->_local_pool
	//! match crew group and segment path
	//! to get columns that have small negetive reduced cost
	//! actually, is assign group for each segpath in _seg_path_searcher._segpath_set
	void matchGroupAndPath();
	//void addToMaster();

	/*output for master problem*/
	
	//! ֻ����һ�Σ��������ʼ����ʱ��
	std::vector<Opt_Segment*>& getCurDaySegSet() { return _cur_day_seg_set; }

	ColumnPool& getCurLocalPool() { return _local_pool; }

private:		
	
	CrewRules* _rules;
	std::vector<Opt_Segment*> _cur_day_seg_set;
	std::vector<CrewNode*> _crewnode_set; //TODO:δ��ʼ��
	ColumnPool _local_pool; //ÿ��������õ�������

	GroupSearcher _group_searcher;
	SegPathSearcher _seg_path_searcher;
	std::vector<SegPath*> _cur_day_path_set;
	
	int _fly_mint_range[2];
	/*std::vector<double> _seg_cover_duals;
	std::vector<double> _crew_assign_duals;
	std::vector<double> _balance_flytime_duals;
	std::vector<double> _balance_resttime_duals;*/


};

#endif // !SUBPROBLEM_H

