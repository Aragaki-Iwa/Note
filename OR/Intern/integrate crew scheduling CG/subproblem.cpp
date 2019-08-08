#include "subproblem.h"

SubProblem::SubProblem(CrewNetwork& crewNet, SegNetwork& segNet, CrewRules& rules) {
	_rules = &rules;
	_group_searcher.init(crewNet, rules);
	_seg_path_searcher.init(segNet, *segNet.resource, rules);

	//for debug ��ʱ������crewNet
	/*_crewnode_set = crewNet.nodeSet;
	_crewnode_set.erase(_crewnode_set.begin(), _crewnode_set.begin() + 2);*/
	
}
SubProblem::~SubProblem() {}


void SubProblem::setInitialGroups(std::vector<CrewGroup*>& initialGroups) {
	_group_searcher.getPathSet() = initialGroups;	
}

void SubProblem::findSegPaths() {
	_seg_path_searcher.search();			

	std::cout << "value " << &_cur_day_path_set;
	_cur_day_path_set = _seg_path_searcher.getPathSet();
	std::cout << "value " << &_cur_day_path_set;

}

void SubProblem::setPathStatus(const Penalty& penalty) {
	std::cout << "value " << &_cur_day_path_set;
	for (auto& path : _cur_day_path_set) {
		path->init(penalty);		
	}
	//��cost��������
	std::sort(_cur_day_path_set.begin(), _cur_day_path_set.end(),
		[](const SegPath* a, const SegPath* b) {return a->getCost() > b->getCost(); });

	_seg_path_searcher.clear();
}

void SubProblem::setCurDaySegSet() {
	for (const auto& segpath : _cur_day_path_set) {
		std::vector<SegNode*>& segnode_set = segpath->getNodeSequence();
		for (const auto& segnode : segnode_set) {
			if (std::find(_cur_day_seg_set.begin(), _cur_day_seg_set.end(), segnode->optSegment) 
				== _cur_day_seg_set.end()) {
				_cur_day_seg_set.emplace_back(segnode->optSegment);
			}
		}
	}
}
void SubProblem::setIndexOfCurSeg() {
	for (auto& path : _cur_day_path_set) {
		path->setSegIndexSet(_cur_day_seg_set);
	}
}

void SubProblem::labelSpecialSegPath(std::vector<std::string>& specialAirports) {
	std::string dep_arp;
	std::string arv_arp;
	for (auto& segpath : _cur_day_path_set) {
		segpath->specialCredentials["SpecialAirport"] = 0;
		auto node_set = segpath->getNodeSequence();
		for (const auto& segnode : node_set) {
			dep_arp = segnode->depStation;
			arv_arp = segnode->arvStation;
			if (std::find(specialAirports.begin(), specialAirports.end(), dep_arp) != specialAirports.end()
				|| std::find(specialAirports.begin(), specialAirports.end(), arv_arp) != specialAirports.end()) {
				segpath->specialCredentials["SpecialAirport"] = +1;
			}
		}
	}
}

void SubProblem::updateDuals(std::vector<double>& segCoverDuals, std::vector<double>& crewAssignDuals) {
	for (size_t i = 0; i < _cur_day_seg_set.size(); i++) {
		_cur_day_seg_set[i]->setDualPrice(segCoverDuals[i]);
	}
	for (size_t i = 0; i < _crewnode_set.size(); i++) {
		_crewnode_set[i]->setPrice(crewAssignDuals[i]);
	}
}
void SubProblem::findGroups() {
	_group_searcher.search();	
}

void SubProblem::matchGroupAndPath() {
	_local_pool.clear();
	auto group_set = _group_searcher.getPathSet();
	std::string special = "SpecialAirport";
	//TODO:���̼߳���
	std::string rest_station;
	for (auto& group : group_set) {
		group->setBasicProperties();
		
		rest_station = group->getCrewGroup().front()->workStatus->restStation;

		int gap_crew_segpath = 0;		
		for (auto& segpath : _cur_day_path_set) {
			SegNode* start_node = segpath->startNode;
			SegNode* end_node = segpath->endNode;
			if (rest_station != "" && rest_station != start_node->depStation) {
				//=="",˵���ǳ��ε���������Ҫ����ռ����������ʵ����crew�ĳ�ʼ״̬������һ���ƻ����ڿ�ʼʱ�����ڵص���Ϣ�������Ȳ�����
				continue;
			}
			//2.���ʱ��Լ��
			gap_crew_segpath = (start_node->startDtLoc - group->endDtLoc) / 60;
			if (gap_crew_segpath <= 0) {
				continue;
			}
			else if (gap_crew_segpath > 72 * 24 * 60) {
				break;
			}
			//���ܵ�����duty����Ϊ�ᳬʱ.dԽ��ʱ��Խ�������Բ��������²��ң�ֱ��break
			else if (0 < gap_crew_segpath && gap_crew_segpath < _rules->minDayOffMin) {
				//����Ҫday off������duty֮�����������
				//��Ԥ���ж�һ����������duty���Ƿ�ᳬʱ
				if (group->max_credit_mint + gap_crew_segpath + segpath->total_credit_mint > _rules->maxCreditMin
					|| group->max_fly_mint + segpath->total_fly_mint > _rules->maxWeekFlyMin) {
					break;
				}
			}
							
			//3.�����ռ��ʱ�䣬�������	
			if (segpath->specialCredentials[special] >= 1 &&
				group->spetialCredentials[special] != 1) { //duty��Ҫ�������ʣ���crew���߱�
				continue;
			}
			
			//�����㣬��segPath�͸�crewGroup���Դ���
			// create new column
			int reduced_cost = segpath->getCost() - segpath->total_dual_price - group->getCost();
			if (reduced_cost <= -1e-8) {
				Column* col = new Column();
				col->_segpath = segpath;
				col->_crewgroup = group;
				col->cost = segpath->getCost();
				col->reduced_cost = reduced_cost;

				_local_pool.emplace_back(col);
			}
			
		}
	}
	
}



int* SubProblem::getFlyMintRange() {
	auto group_set = _group_searcher.getPathSet();
	int max_fly_mint = group_set.front()->max_fly_mint;
	int min_fly_mint = group_set.front()->max_fly_mint;

	for (const auto& group : group_set) {
		auto node_set = group->getNodeSequence();
		for (const auto& node : node_set) {
			int temp_mint = node->optCrew->workStatus->accumuFlyMin;
			max_fly_mint = max_fly_mint > temp_mint
				? max_fly_mint : temp_mint;
			min_fly_mint = min_fly_mint < temp_mint
				? min_fly_mint : temp_mint;
		}
	}

	_fly_mint_range[0] = min_fly_mint;
	_fly_mint_range[1] = max_fly_mint;
	return _fly_mint_range;
}