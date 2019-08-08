#include "CrewGroupSearcher.h"
#include "CrewRules.h"
#include "Crew_Network.h"
#include "Crew_Path.h"
#include "csvClassesTransOptimizable.h"


void GroupSearcher::init(CrewNetwork& crewNet, CrewRules& rules) {
	_net = &crewNet;
	_s = crewNet.resource;
	_t = crewNet.sink;
	_rules = &rules;
};

void GroupSearcher::search() {
	searchByTopo();
	
	for (auto group : _crewgroup_set) {		
		group->setCrewIndexSet();
		group->setCrewGroup();		
		group->setCrewGroup();
		group->setBasicProperties();
		group->computeCost();
	}
}


bool GroupSearcher::checkCrewStatus(const Opt_CREW& optCrew) {
	auto status = optCrew.workStatus;
	return status->accumuFlyMin > _rules->maxWeekFlyMin || status->accumuCreditMin > _rules->maxCreditMin;
}

bool GroupSearcher::searchByTopo() {
	if (_topo_order.empty()) {
		setTopoOrder();
	}
	_crewgroup_set.reserve(20);
	std::make_heap(_crewgroup_set.begin(), _crewgroup_set.end());

	for (const auto& node : _net->nodeSet) {
		_cost_map[node] = _NEGATIVE_INF;
		_prev_map[node] = NULL;
		
		_node_labels[node] = {};
	}
	_cost_map[_s] = 0;
	_prev_map[_s] = _s;
	crewLabel* label_of_s = new crewLabel;
	label_of_s->cur_price = 0;
	label_of_s->cur_node_sequence.emplace_back(_s);
	_node_labels[_s].emplace_back(label_of_s);


	std::vector<Opt_CREW*> combi;
	CrewNode* cur;
	CrewNode* next;
	while (!_topo_order.empty()) {
		cur = _topo_order.front(); 
		_topo_order.pop();
		
		if (_cost_map[cur] != _NEGATIVE_INF) {
			for (auto& arc : cur->outArcSet) {
				next = arc->endNode;

				if (next == _t) { 					
					for (const auto& label : _node_labels[cur]) {
						CrewGroup* group = new CrewGroup();
						group->getNodeSequence() = label->cur_node_sequence;
						group->computeCost();
						
						if (group->getCost() < _crewgroup_set.front()->getCost()) {
							delete group;
							continue;
						}

						_crewgroup_set.emplace_back(group);
						std::push_heap(_crewgroup_set.begin(), _crewgroup_set.end());											
					}

					continue;
				}
				//next != t				
				for (crewLabel* cur_label : _node_labels[cur]) {					
					crewLabel* new_label = new crewLabel;
					//extend
					new_label->cur_node_sequence = cur_label->cur_node_sequence;
					new_label->cur_price = cur_label->cur_price;
					//update
					new_label->cur_node_sequence.emplace_back(next);
					new_label->cur_price += arc->endNode->price;					
					for (auto& node : new_label->cur_node_sequence) {
						if (node != _s && node != _t) {
							combi.emplace_back(node->optCrew);
						}						
					}

					if (new_label->cur_node_sequence.size() > _max_num_path || !_rules->isFasibleCombination(combi)) {
						delete new_label;
						continue;
					}

					if (_cost_map[next] < _cost_map[cur] + cur->price) {
						_cost_map[next] = _cost_map[cur] + cur->price;

						_node_labels[next].emplace_back(new_label);
					}
										
				}
								
			}			
			
			for (size_t i = 0; i < _node_labels[cur].size(); i++) {
				delete _node_labels[cur][i];
			}
			_node_labels[cur].clear();
			_node_labels[cur].shrink_to_fit();
		}
	}

	return true;
}
void GroupSearcher::setTopoOrder() {
	
	std::queue<CrewNode*> temp_queue;
	size_t count = 0;
	//initialize
	for (auto& node : _net->nodeSet) {
		count = node->inArcSet.size();
		_node_indegree[node] = count;
		
		if (count == 0) {
			temp_queue.push(node);
		}
	}
	
	count = 0;
	CrewNode* node;
	while (!temp_queue.empty()) {
		node = temp_queue.front(); temp_queue.pop();
		_topo_order.push(node);
		for (auto& arc : node->outArcSet) {
			if (--_node_indegree[arc->endNode] == 0) {
				temp_queue.push(arc->endNode);
			}
		}
		++count;
	}
	if (count != _net->nodeSet.size()) {
		throw "the network is cyclic!!";
	}	
}

bool GroupSearcher::searchBySPFA() {
	return true;
}

//bool PathSearcher::searchByDijk() {
//	_prior_queue.push(&prioNode(_s, 0));
//	//init
//	INode* cur;
//	INode* next;
//	while (!_prior_queue.empty()) {
//		cur = _prior_queue.top()->inode; _prior_queue.pop();
//		if (cur->visited) {
//			continue;
//		}
//		cur->visited = true;
//		for (auto& arc : cur->outArcSet) {
//			next = arc->endNode;
//			//因为求最长路
//			//所以此处松弛为：经过点cur到达next的cost > 到达next，当前的cost
//			if (!next->visited && _cost_map[next] < _cost_map[cur] + next->price) {
//				_cost_map[next] = _cost_map[cur] + next->price;
//												
//				_prev_map[next] = cur;
//
//				_prior_queue.push(&prioNode(next, _cost_map[next]));
//			}
//
//		}
//	}
//
//}


