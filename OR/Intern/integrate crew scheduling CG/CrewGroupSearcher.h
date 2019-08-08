#pragma once
#ifndef CREW_GROUP_SEARCHER_H
#define CREW_GROUP_SEARCHER_H
#include "pch.h"

class CrewNetwork;
class CrewGroup;
class CrewNode;
class Opt_CREW;

class CrewRules;

class GroupSearcher
{
public:
	//GroupSearcher(CrewNetwork& crewNet, CrewNode& s, CrewNode& t, CrewRules& rules) {
	//	/*_net = &crewNet; 
	//	_s = &s; 
	//	_t = &t;
	//	_rules = &rules;*/
	//};
	~GroupSearcher() {
		_net = NULL;
		_s = NULL;
		_t = NULL;
		_rules = NULL;
		_crewgroup_set.clear();
	};

	//void setRules(CrewRules& rules) { _rules = &rules; }	
	//! Set private members
	//! _net; _s; _t; 
	//void inputCrewNetwork(CrewNetwork* crewNet, CrewNode* s, CrewNode* t) { _net = crewNet; _s = s; _t = t; }
	
	void init(CrewNetwork& crewNet, CrewRules& rules);
	void search();
	//! 在挑选crewGroup的时候检查crew的工作状态
	bool checkCrewStatus(const Opt_CREW& optCrew);

	std::vector<CrewGroup*>& getPathSet() { return _crewgroup_set; }
	

private:
	//search for seg path
	//seg net only cover 1 day, so enumerate all feasible path by dfs is not time-consuming
	//bool searchByDFS();
	
	// !net must be a DGA
	bool searchByTopo();
	void setTopoOrder();
	//search for crew group
	//crew network might be a negetive gragh 
	bool searchBySPFA();
	//search for crew group	
	//exactly search for longest path
	//bool searchByDijk();

	/*INetwork* _net;
	INode* _s;
	INode* _t;*/

	/*----------------------Crew Network----------------------*/
	CrewNetwork* _net;
	CrewNode* _s;
	CrewNode* _t;
	CrewRules* _rules;

	const int _NEGATIVE_INF = 0xc0c0c0c0;
	std::map<CrewNode*, double> _cost_map;
	std::map<CrewNode*, CrewNode*> _prev_map;

	/**topo**/
	struct crewLabel
	{
		std::vector<CrewNode*> cur_node_sequence;
		double cur_price;
	};
	std::unordered_map<CrewNode*, std::vector<crewLabel*>> _node_labels; //是否需要保留多个label
	std::unordered_map<CrewNode*, int> _node_indegree;
	std::queue<CrewNode*> _topo_order;
	/**end! topo**/

	/**combination rules**/
	//! 路径最多包含的Node数（不包含虚拟起终点） = 最大号位数
	//! 用来剪枝
	int _max_num_path = 4;
	/**end! combination rules**/

	std::vector<CrewNode*> _node_sequence;
	std::vector<CrewGroup*> _crewgroup_set;

	
	/*----------------------end! Crew Network----------------------*/
	
	///*Dijkstra*/
	//struct prioNode
	//{
	//	prioNode(INode* inode, double dist) {
	//		this->inode = inode;
	//		this->dist = dist;
	//	}
	//	INode* inode;
	//	double dist;
	//	bool operator()(const prioNode* pnode1, const prioNode* pnode2) {
	//		return pnode1->dist < pnode2->dist;
	//	}
	//};
	//std::priority_queue<prioNode*> _prior_queue;	
	///*end! Dijkstra*/
	
	/*std::vector<INode*> _node_sequence;	
	std::vector<IPath*> _path_set;*/

};


#endif // !CREW_GROUP_SEARCHER_H