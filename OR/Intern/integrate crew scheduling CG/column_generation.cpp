#include "column_generation.h"

ColumnGeneration::ColumnGeneration() {
}
ColumnGeneration::~ColumnGeneration() {

}

void ColumnGeneration::init(/*ColumnPool& initialPool*/std::vector<CrewGroup*>& initialGroups,
							CrewNetwork& crewNet,
							SegNetwork& segNet,
							CrewRules& rules,
							const Penalty& penaltySetting) {
	//_global_pool = &initialPool;
	_global_pool = new ColumnPool();
	_crew_net = &crewNet;
	_seg_net = &segNet;
	_rules = &rules;
	_penalty = &penaltySetting;
	_crew_node_set = &crewNet.nodeSet;
	_seg_node_set = &segNet.nodeSet;

	 _sub_pro = new SubProblem(*_crew_net, *_seg_net, *_rules);
	 _sub_pro->setInitialGroups(initialGroups);
	 _master = new MasterProblem();	 

	_lb = 0;
	_ub = MAX;
	_max_num_iter = MAX_NUM_ITER;
}

void ColumnGeneration::solve() {
	int result = 0;	
	_sub_pro->findSegPaths();
	_sub_pro->setPathStatus(*_penalty);
	_sub_pro->setCurDaySegSet();
	_sub_pro->setIndexOfCurSeg();
	//create initial column pool
	_sub_pro->matchGroupAndPath();

	_global_pool->swap(_sub_pro->getCurLocalPool()); //sub_pro中的列池变为空

	_master->init(_global_pool, _sub_pro->getCurDaySegSet(), *_crew_node_set); //NOTE：后两个参数只需要传一次，改！
	_master->initSetting();
	_master->initParameters();
	_master->addRestColumns();


	_master->buildModel();
	for (size_t i = 1; i < _max_num_iter; i++) {				
		//debug
		//std::cout << std::to_string(_master->getPathDvar().isElementsType(IloNumVar::Float)) << std::endl;
		
		//_master->exportModel();

		result = _master->solve();
		//_master->writeSoln();

		_lb = _master->getObjValue();

		if (i % _frequency_solve_mip == 0 && result == 1) {
			solveMIP();
			double obj_value_mip = _mip_cplex.getObjValue();
			Solution* new_soln = new Solution();
			getFeasibleSolution(new_soln);
			_solution_pool.emplace_back(new_soln);
			if (obj_value_mip < _ub) {
				_ub = obj_value_mip;
				_best_solution_pool.clear();
				_best_solution_pool.emplace_back(new_soln);
			}
			else if (obj_value_mip == _ub) {
				_best_solution_pool.emplace_back(new_soln);
			}
		}

		_sub_pro->updateDuals(_master->getSegCoverDuals(), _master->getCrewAssignDuals());
		_sub_pro->findGroups();		
		_sub_pro->matchGroupAndPath();
		//if(reduce cost >= 0)

		_master->addNewColumns(_sub_pro->getCurLocalPool());
	}
}
void ColumnGeneration::solveMIP() {
	_mip_model.end();
	_mip_cplex.end();

	IloEnv env = _master->_model.getEnv();
	_mip_model = IloModel(env);	
	_mip_model.add(_master->_model);
	_mip_model.add(IloConversion(env, _master->getPathDvar(), IloNumVar::Type::Int));
	_mip_model.add(IloConversion(env, _master->getUncoverDvar(), IloNumVar::Type::Int));
	
	_mip_cplex = IloCplex(_mip_model);	
	_mip_cplex.solve();
}
void ColumnGeneration::getFeasibleSolution(Solution* soln) {
	IloNumArray values(_mip_cplex.getEnv());
	_mip_cplex.getValues(values, _master->getPathDvar());
	//std::vector<double> dvar_values(values.getSize());	
	for (size_t i = 0; i < values.getSize(); i++) {
		if (values[i] > 0) {
			soln->column_pool->emplace_back((*_global_pool)[i]);
		}
	}	
}

