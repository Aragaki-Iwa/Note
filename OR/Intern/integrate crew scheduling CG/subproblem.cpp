#include "subproblem.h"
#include "algorithm_xdb.h"


/*DEBUG 8-15 find uncover seg and possible crew*/
//const std::vector<long long> kUNCOVER_SEG_ID = { 54266023, 54253231, 54246335,54266024,54267951 };

const std::map<long long, std::vector<std::string> > kUNCOVER_SEG_POSSIBLE_CREW = {
			{54266023,{"001963","003827","000547"}},
			{54253231,{"000783", "002442"}},
			{54246335,{"000323","002216", "001621", "001880", "001745", "003822"}},
			{54266024,{"000619","001826","000925","000697","001780","003509"}},
			{54267951,{"000966","000932","000696","001651","001781","000102","002825","001874","001150","000302","001758","003209"}} };

/*end! DEBUG 8-15 find uncover seg and possible crew*/

SubProblem::SubProblem(CrewNetwork& crewNet, SegNetwork& segNet, CrewRules& rules) {
	_rules = &rules;
	_group_searcher.init(crewNet, rules);
	_seg_path_searcher.init(segNet, *segNet.resource, rules);


	//for debug ��ʱ������crewNet
	_crewnode_set = crewNet.nodeSet;
	_crewnode_set.erase(_crewnode_set.begin(), _crewnode_set.begin() + 2);
	
}
SubProblem::~SubProblem() {

}


void SubProblem::setInitialGroups(std::vector<CrewGroup*>& initialGroups) {
	_group_searcher.getPathSet() = initialGroups;	
}

void SubProblem::findSegPaths() {
	_seg_path_searcher.search();	
	_cur_day_path_set = _seg_path_searcher.getPathSet();

}

void SubProblem::setPathStatus(const Penalty& penalty) {	
	for (auto& path : _cur_day_path_set) {
		path->computeCost(penalty);	
	}
	////��cost��������
	//std::sort(_cur_day_path_set.begin(), _cur_day_path_set.end(),
	//	[](const SegPath* a, const SegPath* b) {return a->getCost() > b->getCost(); });

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

void SubProblem::estimateMeanFlyMin() {
	//ȡsegpath_set���м�80%���Լ�crew_set���м�80%�����ʱ��ֵ

	std::sort(_cur_day_path_set.begin(), _cur_day_path_set.end(),
		[](const SegPath* a, const SegPath* b) {return a->total_fly_mint > b->total_fly_mint; });
	int low = 0.1 * _cur_day_path_set.size();
	int high = 0.9 * _cur_day_path_set.size();

	int mean_segpath_fly_min = 0;
	for (size_t i = low; i < high; i++) {
		mean_segpath_fly_min += _cur_day_path_set[i]->total_fly_mint;
	}
	mean_segpath_fly_min /= (high - low + 1);

	//! changed 8-12-2019
	//! ��Ӧ�øı�_crewnode_set��˳����Ϊ֮��Ķ�ż�۸�Ҫ��Ӧ 
	std::vector<CrewNode*> temp_crewnode_set(_crewnode_set);

	std::sort(temp_crewnode_set.begin(), temp_crewnode_set.end(),
		[](const CrewNode* a, const CrewNode* b) {return a->optCrew->workStatus->accumuFlyMin < b->optCrew->workStatus->accumuFlyMin; });
	low = 0.1 * (temp_crewnode_set.size());
	high = 0.9 * (temp_crewnode_set.size());
	int mean_crew_fly_min = 0;
	for (size_t i = low; i < high; i++) {
		mean_crew_fly_min += temp_crewnode_set[i]->optCrew->workStatus->accumuFlyMin;
	}
	mean_crew_fly_min /= (high - low + 1);

	_mean_fly_mint = mean_segpath_fly_min + mean_crew_fly_min;
}


void SubProblem::updateDuals(std::vector<double>& segCoverDuals, std::vector<double>& crewAssignDuals) {
	for (size_t i = 0; i < _cur_day_seg_set.size(); i++) {
		_cur_day_seg_set[i]->setDualPrice(segCoverDuals[i]);
	}
	for (auto& path : _cur_day_path_set) {
		path->total_dual_price = 0;
		auto node_of_path = path->getNodeSequence();
		for (auto& node : node_of_path) {
			path->total_dual_price += node->optSegment->getDualPrice();
		}
	}

	for (size_t i = 0; i < _crewnode_set.size(); i++) {
		_crewnode_set[i]->setPrice(crewAssignDuals[i]);
	}
}
void SubProblem::findGroups() {
	_group_searcher.search();	
}

void SubProblem::matchGroupAndPath() {
	//_local_pool.clear();
	auto group_set = _group_searcher.getPathSet();
	std::string special = "SpecialAirport";
	//TODO:���̼߳���
	std::string rest_station;
	for (auto& group : group_set) {
		//group->setBasicProperties();
		
		rest_station = group->getCrewGroup().front()->workStatus->restStation;

		int gap_crew_segpath = 0;		
		for (auto& segpath : _cur_day_path_set) {
			SegNode* start_node = segpath->startNode;
			SegNode* end_node = segpath->endNode;
			
			
			/*DEBUG 8-15 find uncover seg and possible crew*/
			SegNodeSet& segnode_set = segpath->getNodeSequence();
			
			for (size_t s = 0; s < segnode_set.size(); s++) {
				long long seg_id = segnode_set[s]->optSegment->getDBId();
				/*if (isContain(kUNCOVER_SEG_ID, seg_id)) {
					int yy = 0;
				}*/
				auto pos = kUNCOVER_SEG_POSSIBLE_CREW.find(seg_id);
				if (pos != kUNCOVER_SEG_POSSIBLE_CREW.end()) {
					for (const auto& crew : group->getCrewGroup()) {
						const std::vector<std::string>& possible_crews = kUNCOVER_SEG_POSSIBLE_CREW.at(seg_id);
						if (isContain(possible_crews, crew->getIdCrew())) {
							int yy = 0;
						}
					}
				}
			}
			
			
			/*end! DEBUG 8-15 find uncover seg and possible crew*/
			
			
			
			if (rest_station != "" && rest_station != start_node->depStation) {
				//=="",˵���ǳ��ε���������Ҫ����ռ����������ʵ����crew�ĳ�ʼ״̬������һ���ƻ����ڿ�ʼʱ�����ڵص���Ϣ�������Ȳ�����
				continue;
			}

			//�������Ƿ����
			if (group->compo_mode->name  != segpath->getCompoMode()->name) {
				continue;
			}

			//2.���ʱ��Լ��
			gap_crew_segpath = (start_node->startDtLoc - group->endDtLoc) / 60;
			if (gap_crew_segpath <= 0) {
				continue;
			}
			//����duty��crew֮��ļ����ʵ���Ͼ�������crew������Ϣ̫��
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
			int balance_cost = calBalanceCost(*group);
			int reduced_cost = segpath->getCost() + balance_cost - segpath->total_dual_price - group->getCost();
			if (reduced_cost <= -1e-8) {
				Column* col = new Column(); //TODO:8-15 ȡ��reduced cost��С�ģ�top k�����ô󶥶ѣ�ֻ��С�ڶѶ���new
				col->_segpath = segpath;
				col->_crewgroup = group;
				col->cost = segpath->getCost() + balance_cost;
				col->reduced_cost = reduced_cost;

				_local_pool.emplace_back(col);
			}
			
		}
	}	
}
void SubProblem::addRestColumns() {
	for (size_t i = 0; i < _crewnode_set.size(); i++) {
		Column* rest_col = new Column();
		rest_col->type = ColumnType::relax;
		CrewGroup* single_crew = new CrewGroup();
		single_crew->getNodeSequence().emplace_back(_crewnode_set[i]);
		single_crew->setCrewIndexSet();
		single_crew->setCrewGroup();
		single_crew->setBasicProperties();
		single_crew->computeCost();

		SegPath* segpath = new SegPath();
		//segpath->_total_cost = 0;	
		segpath->total_fly_mint = 0;
		segpath->total_credit_mint = 0;
		segpath->total_dhd = 0;
		

		rest_col->_segpath = segpath;
		rest_col->_crewgroup = single_crew;
		rest_col->cost = segpath->getCost() + calBalanceCost(*single_crew);

		_local_pool.emplace_back(rest_col);
		
	}

}

//! 8-15 ���㷽ʽ���ԣ�Ӧ���� ����group��dutyƥ�䣬��ô�õ���groupƥ���duty��ķ�ʱ���������ʱ��ȥ��ֵ�Ŷԣ��������õ�ǰgroup�ķ�ʱ��ȥ��ֵ
int SubProblem::calBalanceCost(CrewGroup& crewGroup) {
	int total_divation = 0;
	auto crew_set = crewGroup.getCrewGroup();	
	for (size_t i = 0; i < crew_set.size(); i++) {
		total_divation += std::abs(crew_set[i]->workStatus->accumuFlyMin - _mean_fly_mint);
	}

	return total_divation;
}

//int* SubProblem::getFlyMintRange() {
//	auto group_set = _group_searcher.getPathSet();
//	int max_fly_mint = group_set.front()->max_fly_mint;
//	int min_fly_mint = group_set.front()->max_fly_mint;
//
//	for (const auto& group : group_set) {
//		auto node_set = group->getNodeSequence();
//		for (const auto& node : node_set) {
//			int temp_mint = node->optCrew->workStatus->accumuFlyMin;
//			max_fly_mint = max_fly_mint > temp_mint
//				? max_fly_mint : temp_mint;
//			min_fly_mint = min_fly_mint < temp_mint
//				? min_fly_mint : temp_mint;
//		}
//	}
//
//	_fly_mint_range[0] = min_fly_mint;
//	_fly_mint_range[1] = max_fly_mint;
//	return _fly_mint_range;
//}