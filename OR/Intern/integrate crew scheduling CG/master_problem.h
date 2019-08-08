#pragma once
#ifndef MASTER_PROBLEM_H
#define MASTER_PROBLEM_H
#include "column.h"
#include "Crew_Network.h"
#include "Seg_Network.h"
#include "csvClassesTransOptimizable.h"
#include "ilcplex\ilocplex.h"
#include <memory>

const int PENALTY_UNCOVER = 7000;

class MasterProblem 
{
public:
	MasterProblem();		
	~MasterProblem();

	void initSetting();
	void addRestColumns();
	
	void init(ColumnPool* columnpool_ptr, std::vector<Opt_Segment*>& segSet, std::vector<CrewNode*>& crewNodeSet);
	//! 记录crew的初始飞时
	void initParameters();
	void buildModel();
	void exportModel();	
	int solve();
	void writeSoln();
	double getObjValue() const { return _objValue; }
	std::vector<double>& getSegCoverDuals();
	std::vector<double>& getCrewAssignDuals();
	
	IloNumVarArray& getPathDvar() { return _dvars_path; }
	IloNumVarArray& getUncoverDvar() { return _dvars_uncover; }

	//void reset();
	void addNewColumns(ColumnPool& columnpool_ptr);

	ColumnPool* global_pool; //主问题空间的列集合
	IloModel _model;
	
private:
	//! end all except _env
	void end();
	void addObjFunction();
	//void addConstraints();
	void addConstraints2();
	/*void sortByFlyTime(ColumnPool* columnpool_ptr, std::vector<Opt_CREW*>& crewSet);
	void sortBySegNums();*/

	IloEnv _env;
	IloCplex _cplex;
	
	IloObjective _obj;
	double _objValue;
	IloNumVarArray _dvars_path; //路径决策变量
	IloNumVarArray _dvars_uncover; //路径决策变量

	/*IloNumVar _dvar_upper_flytime;
	IloNumVar _dvar_lower_flytime;*/
	/*IloNumVar _dvar_upper_seg_num;
	IloNumVar _dvar_lower_seg_num;*/

	IloRangeArray _constraints_all;
	IloRangeArray _constraints_segcover;
	IloRangeArray _constraints_crewassign;
	/*IloRangeArray _constraints_blance_u;
	IloRangeArray _constraints_blance_l;*/

	std::map<int, IloRange> _seg_index_cover_constraint;
	std::map<int, IloRange> _crew_index_assign_constraint;

	//std::vector<double> cost;
	std::vector<int> _init_crew_fly_mint;

	std::vector<Opt_Segment*>* _seg_set;
	std::vector<CrewNode*>* _crewnode_set;
	size_t _seg_num;
	size_t _crew_num;

	std::vector<double> seg_cover_duals;
	std::vector<double> crew_assign_duals;
};


#endif // !MASTER_PROBLEM_H