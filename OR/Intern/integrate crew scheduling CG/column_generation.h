#pragma once
#ifndef COLUMN_GENERATION_H
#define COLUMN_GENERATION_H
#include "pch.h"
#include "master_problem.h"
#include "subproblem.h"
#include"ilcplex\ilocplex.h"

const int MAX = 1000 * 1000 * 1000;
const int MAX_NUM_ITER = 300;
const int FREQUENCY_SOLVE_MIP = 20; //每FREQUENCY_SOLVE_MIP次迭代就求一次整数解

class Solution 
{
public:
	ColumnPool* column_pool;
	double obj_value;
};

class ColumnGeneration
{
public:
	ColumnGeneration();
	~ColumnGeneration();

	void init(/*ColumnPool& initialPool*/std::vector<CrewGroup*>& initialGroups, 
			  CrewNetwork& crewNet, 
			  SegNetwork& segNet, 
			  CrewRules& rules,
			  const Penalty& penaltySetting);
	void solve();

	void solveMIP();
	
private:
	void getFeasibleSolution(Solution* soln);

	CrewNetwork* _crew_net;
	SegNetwork* _seg_net;
	CrewRules* _rules;
	const Penalty* _penalty;
	std::vector<CrewNode*>* _crew_node_set;
	std::vector<SegNode*>* _seg_node_set;

	MasterProblem* _master;
	SubProblem* _sub_pro;
	IloModel _mip_model;
	IloCplex _mip_cplex;

	std::vector<Solution*> _solution_pool; //整数解解池
	std::vector<Solution*> _best_solution_pool; //最好解池

	ColumnPool* _global_pool;
	double _lb;
	double _ub;
	int _max_num_iter;
	int _frequency_solve_mip = FREQUENCY_SOLVE_MIP;
};
#endif // !COLUMN_GENERATION_H