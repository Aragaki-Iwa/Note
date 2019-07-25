#pragma once
#include "pch.h"
#include "ilcplex/ilocplex.h"
#include "FindDutySet.h"
#include "..\csvReader\crewDB_mine.h"
#ifndef  CPLEX_MODEL_H
#define CPLEX_MODEL_H

class dutyModel
{
public:
	dutyModel() {
		_model = IloModel(_env, "initial_dutyModel");
		_dutyVarArr = IloNumVarArray(_env);		
		/*_fltCancelVarArr = IloNumVarArray(_env);
		_crewFreeVarArr = IloNumVarArray(_env);*/

		/*_upperNumCoverVar = IloNumVar(_env, _upper_num_lower, _upper_num_upper, IloNumVar::Type::Int);
		_lowerNumCoverVar = IloNumVar(_env, 1, _upper_num_lower, IloNumVar::Type::Int);*/

		_dhdVarArr = IloNumVarArray(_env);
		//_crewDutyVarMatrix = IloArray<IloNumVarArray>(_env);
		_obj = IloObjective(_env);
		_constraints = IloRangeArray(_env);

		_cur_segSet.clear();
		_cur_segSet.shrink_to_fit();

		_decided_dutySet.clear();
		_decided_dutySet.shrink_to_fit();
	};
	~dutyModel() {
		_env.end();
		//delete []coverArray;
	}
	void solve();

	void checkCrewStatus(std::vector<CREW*>& crewSet, const CrewRules& rules);
	
	//crewSet暂时只包括飞行员，所以提前筛选好
	void buildModel(const Network& net, std::vector<Path*>& pathSet, std::vector<CREW*>& crewSet, 
		std::vector<string>& specialAirport,
		const CrewRules& rules);
	
	//后处理：确定当前排班方案后所作的信息更新等
	void post_process(/*Network& net, */std::vector<Path*>& pathSet, std::vector<CREW*>& crewSet);
	std::vector<Path*>* getDecidedDutySet();
	std::vector<Node*>* getCurSegNodeSet();

	void reset(int iter);
private:
	void init(/*const std::vector<Node*>& segSet,*/ std::vector<string>& specialAirport,
		std::vector<Path*>& pathSet/*,std::vector<CREW*>& crewSet*/);
	
	void setCurSegSet(std::vector<Path*>& pathSet);
	
	bool checkRankMatch(CREW_RANK* cap, CREW_RANK* fo, const std::unordered_set<string>& rankCombinationSet);

	void updateCrewStatus();

	std::vector<Node*> _cur_segSet;

	IloEnv _env;
	IloModel _model;
	IloCplex _cplex;
	IloObjective _obj;
	IloNumVarArray _dutyVarArr;	
	IloNumVarArray _fltCancelVarArr;
	IloNumVarArray _crewFreeVarArr;

	IloNumVarArray _dhdVarArr;

	typedef IloArray<IloNumVarArray> CrewDutyVarMatrix;
	CrewDutyVarMatrix _crewDutyVarMatrix;

	IloNumVar _upperNumCoverVar;
	int _upper_num_lower = 3, _upper_num_upper = 5;
	IloNumVar _lowerNumCoverVar;
	
	int _penaltyFltCancel = 2;// 50;
	int _penaltyDhd = 10;
	int _penaltyCrewFree = 10;
	int _specialSegGatherWight = 0;
	int _costDuty = 1;//20;

	IloRangeArray _constraints;
	//int* coverArray;
	std::map<int, int> _numSpecialFltOfDuty;

	std::vector<Path*> _decided_dutySet;
};

#endif // ! CPLEX_MODEL_H

