#include "IntegrateOptimization.h"

template<class T>
void permutations(std::vector<T>& data, int begin, int end, std::vector<T>* permSet) {
	if (begin == end) {
		permSet->resize(end);

	}

}


std::vector<CREW*> RollingOpt::getCrewSet() {
	return _crewSet;
}

void RollingOpt::optimize() {
	
	_net->createNetwork(_segSet, _baseSet, *_rules);
	Node* resource = _net->nodeSet->front();
	for (auto& crew : _crewSet) {
		crew->workStatus->endDtLoc = resource->startDtLoc;
	}
	std::cout << utcToUtcString(resource->startDtLoc);

	std::cout << "----------create Network finished----------\n";	

	initialStartNodeSet();
	_assigner.init(&_crewSet, _rules, &_pos_order_seqs, &_order_pos_seqs, &_curCAPCrewSet, &_curFOCrewSet, &_curAttendantCrewSet);

	std::vector<Node*> coveredNodes;
	int num_iter = 1;
	try {
		while (!termination() && num_iter < 10) {
			std::cout << "----------" << num_iter << "th iter started----------\n";
			if (num_iter == 3) {
				int yyy = 7;
			}			
			
			/*enumerate duty*/
			_pathFinder.findPathSet(_net, _cur_start_nodeSet, *_rules);
			_cur_dutySet = *_pathFinder.getPathSet();//此处注意pathFinder中pathSet的内存释放
			
			std::cout << "find " << _cur_dutySet.size() << " dutys\n";
			//删除掉只有一个segment，且为dhd的duty
			removeDhdDuty(_cur_dutySet);
			std::cout << "after remove one-dhd dutys,still " << _cur_dutySet.size() << " dutys\n";

			/*set covering: get decidedDutySet*/
			_dutyModel.buildModel(*_net, _cur_dutySet, _crewSet, _special_airportSet, *_rules);
			_dutyModel.solve();
			_dutyModel.post_process(_cur_dutySet, _crewSet);					
			_dutyModel.reset(num_iter); 
			
			/*assignment*/
			_cur_dutySet = *_dutyModel.getDecidedDutySet();
			std::vector<Node*>* curSegSet = _dutyModel.getCurSegNodeSet();

			_assigner.receiveSetCoverInput(&_cur_dutySet, curSegSet);
			_assigner.labelDecidedDuty(_special_airportSet);
			
			_assigner.solve();
			_cur_dutySet = *_assigner.getAssignedDutySet();
			
			_decided_dutySet.insert(_decided_dutySet.end(), _cur_dutySet.begin(), _cur_dutySet.end());
			
			for (const auto& duty : _decided_dutySet) {
				for (const auto& node : duty->route) {
					if (node->nodeType == seg && std::find(coveredNodes.begin(), coveredNodes.end(), node) == coveredNodes.end()) {
						coveredNodes.emplace_back(node);
					}
				}
			}
			std::cout << "----------covered node until now: " << coveredNodes.size() << "----------\n\n";
			std::cout << "----------" << num_iter++ << "th iter finished----------\n\n";
			updateStartNodeSet(num_iter);
		}

		/*ofstream uncoveredFlts;
		uncoveredFlts.open("uncoveredFlights_output.txt", std::ios::out);
		flightParser flt_csv_parser;
		std::vector<string> headers = flt_csv_parser.getDefaultHeaders();
		for (const auto& col : headers) {
			uncoveredFlts << col << ",";
		}
		uncoveredFlts << "\n";*/
		int num = 0;
		for (const auto& node : *_net->nodeSet) {
			if (node->nodeType == NodeType::seg /*&& node->assigned == false*/) {
				//uncoveredFlts << flt_csv_parser.toCsv(headers, node->segment);
				num++;
			}			
		}
		//uncoveredFlts.close();

		std::cout << "num of seg " << num << "\n";

		std::vector<Node*> coveredNodes;
		for (const auto& crew : _crewSet) {
			auto crew_dutySet = crew->workStatus->CreditedDutySet;
			for (const auto& duty : crew_dutySet) {
				for (const auto& node : duty->route) {
					if (node->nodeType == NodeType::seg && node->assigned 
						&& std::find(coveredNodes.begin(), coveredNodes.end(),node) == coveredNodes.end()) {
						coveredNodes.emplace_back(node);
					}
				}
			}
		
		}
		
		std::cout << "covered node num " << coveredNodes.size() << "\n"  ;


	}
	catch (IloException& e) {
		std::cout << "iloException: " << e.getMessage() << "\n";
	}
	catch (...) {
		std::cerr << "<<<<<<<<<<<<<<<<<<<< OPTIMIZE FAILED >>>>>>>>>>>>>>>>>>>>" << std::endl;
		std::cerr << "---------------Unknown exception caught---------------" << std::endl;
	}
	
}

void RollingOpt::removeDhdDuty(std::vector<Path*>& dutySet) {
	for (auto iter = dutySet.begin(); iter != dutySet.end();) {
		if ((*iter)->route.size() == 1 && (*iter)->route.back()->nodeType == NodeType::dhd) {
			iter = dutySet.erase(iter);
		}
		else {
			iter++;
		}
	}
}

void RollingOpt::initialStartNodeSet() {
	//_cur_start_nodeSet.emplace_back(_net->nodeSet->front());
	//取第一天的点
	auto first_node = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size();
	time_t first_day = getStartTimeOfDay((*first_node)->startDtLoc);
	time_t node_inDay;

	for (auto iter = first_node; iter != _net->nodeSet->end(); iter++) {
		node_inDay = getStartTimeOfDay((*iter)->startDtLoc);
		if (node_inDay == first_day) {
			_cur_start_nodeSet.emplace_back((*iter));
		}
	}
}

bool RollingOpt::termination() {
	//从后往前扫描net
	for (const auto& node : *_net->nodeSet) {
		if (!node->assigned) {
			return false;
		}
	}
	return true;
}
void RollingOpt::updateStartNodeSet(int iterDay) {
	_cur_start_nodeSet.clear();
	//1.上一阶段duty结束点（segment）的后继节点
	//2.加上，在所有结束点中最晚的点之前出发的，未被assign的segment
	time_t last_arv = 0;
	Node* last;
	Node* end;
	for (const auto& duty : _cur_dutySet) {
		end = duty->route.back();
		for (auto& arc : *end->outArcSet) {
			if ((arc->endNode->nodeType == NodeType::seg || arc->endNode->nodeType == NodeType::dhd)
				&& arc->endNode->assigned == false 
				&& std::find(_cur_start_nodeSet.begin(), _cur_start_nodeSet.end(), arc->endNode) == _cur_start_nodeSet.end()) {
				_cur_start_nodeSet.emplace_back(arc->endNode);
			}
		}

		if (end->endDtUtc > last_arv) {
			last_arv = end->endDtUtc;
			last = end;
		}
	}

	auto last_index = std::find(_net->nodeSet->begin(), _net->nodeSet->end(), last);
	auto seg_begin = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size();
	for (auto iter = last_index; iter != seg_begin; iter--) {
		if ((*iter)->nodeType == NodeType::seg && (*iter)->assigned == false && (*iter)->startDtUtc <= last_arv) {
			_cur_start_nodeSet.emplace_back(*iter);
		}
	}
	end = nullptr;
	//3.当前阶段对应天出发的点	
	time_t cur_startDay = _net->nodeSet->front()->startDtUtc + 3600 * 24 * (iterDay - 1);
	time_t node_inDay;
	for (auto iter = _net->nodeSet->begin() + 2 + 2 * _net->baseSet->size(); iter != _net->nodeSet->end(); iter++) {
		node_inDay = getStartTimeOfDay((*iter)->startDtLoc);
		if (node_inDay == cur_startDay && (*iter)->assigned == false
			&& std::find(_cur_start_nodeSet.begin(), _cur_start_nodeSet.end(), (*iter)) == _cur_start_nodeSet.end()) {
			_cur_start_nodeSet.emplace_back(*iter);
		}
		else if (node_inDay > cur_startDay) {
			break;//因为node是按排序来的
		}
	}
}

//data input
void RollingOpt::inputData(std::map<string, std::vector<void*>>& dataSet, const std::vector<string>& objNameSet) {
	for (const auto& obj_name : objNameSet) {
		if (obj_name == "Flight") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_segSet); }
		else if (obj_name == "Base") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_baseSet); }
		else if (obj_name == "Crew") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crewSet); }
		else if (obj_name == "CrewRank") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crew_rankSet); }
		else if (obj_name == "CrewBase") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_crew_baseSet); }
		else if (obj_name == "FlightComposition") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_fltCompositionSet); }
		else if (obj_name == "Composition") { _inputHandler.typeTrans(dataSet[obj_name], obj_name, &_compositionSet); }
		else { std::cerr << "ERROR: invalid obj data name" << obj_name << std::endl; }
	}
}
void RollingOpt::setRules(CrewRules& rules) {
	_rules = &rules;
	rules.rankCombinationSet = _rankCombinationSet;
}
void RollingOpt::init() {
	/*_crewSet = *_inputHandler.getPilotSet(_crewSet);
	_special_airportSet = _inputHandler.*/	
	_inputHandler.matchSegmentAndComposition(&_segSet, &_fltCompositionSet, &_compositionSet);
	_inputHandler.matchCrewAndRank(&_crewSet, &_crew_rankSet);
	_inputHandler.matchCrewAndBase(&_crewSet, &_crew_baseSet);
	for (auto& crew : _crewSet) {
		_inputHandler.sortCrewRank(crew->rankAry);
	}

	_crewSet = *_inputHandler.getPilotSet(_crewSet);

	clusterCrewMeb();
}
void RollingOpt::clusterCrewMeb() {
	string crew_division;
	CREW* crew;
	for (int c = 0; c < _crewSet.size(); c++) {
		crew = _crewSet[c];
		crew_division = crew->division;
		if (crew_division == "P") {
			if (crew->rankAry->front()->rank == "CAP") {
				_curCAPCrewSet.emplace_back(crew);
			}
			else if (crew->rankAry->front()->rank == "FO") {
				_curFOCrewSet.emplace_back(crew);
			}
		}
		else if (crew_division == "C") {
			_curAttendantCrewSet.emplace_back(crew);
		}
	}
}

void RollingOpt::randomSet_specialAirport() {
	_inputHandler.getAirportSet(_segSet);
	_special_airportSet = *_inputHandler.createSpecialArpSet();
	//debug
	std::cout << "-----------special airport-----------\n";
	for (const auto& arp : _special_airportSet) {
		std::cout << arp << "\n";
	}
}
void RollingOpt::randomSet_crewSkills(double percent) {
	string skill("SpecialAirport");	
	int num_special_crew = 0;
	int size = _crewSet.size();
	CREW_RANK* rank;
	std::cout << "-----------special airport-----------\n";
	for (auto& crew : _crewSet) {
		rank = (*crew->rankAry)[0];
		rank->SkillSet.clear();
		if (rand() % 2 == 1) {
			rank->SkillSet[skill] = 1;
			++num_special_crew;

			std::cout << "crew <" << crew->idCrew << "> able to fly: " << skill << "\n";
		}

		if (num_special_crew / size >= percent) {
			break;
		}
	}
}
void RollingOpt::set_rankCombination() {
	std::cout << "----------Rank Combinations----------\n";
	stringstream combi;
	#pragma region //搭配组合的全排列		
	std::vector<string> CAP_permus2;
	std::vector<string> FO_permus2;
	/*1 1CAP-{1FO/2FO}*/
	//1.1 1CAP1FO
	auto set1 = &CAP_positions;
	auto set2 = &FO_positions;
	int num = 0;
	while (num < 2) {
		for (const auto& pos1 : *set1) {
			for (const auto& pos2 : *set2) {
				if ((pos1.find("K1") != string::npos && pos2.find("F6") != string::npos)
					|| (pos1.find("F6") != string::npos && pos2.find("K1") != string::npos)) {
					continue;
				}
				
				combi.str("");	
				combi << pos1 << "-";
				combi << pos2;
				_rankCombinationSet.emplace_back(combi.str());
			}
		}
		std::swap(set1, set2);
		num++;
	}
	//1.2 1CAP2FO
	//得到2FO的所有排列
	for (int i = 0; i < FO_positions.size(); i++) {
		for (int j = 0; j < FO_positions.size(); j++) {
			combi.str("");
			combi << FO_positions[i] << "-";
			combi << FO_positions[j];			
			FO_permus2.emplace_back(combi.str());
		}
	}
	//1CAP与2FO排列
	set1 = &CAP_positions;
	set2 = &FO_permus2;
	num = 0;
	while (num < 2) {
		for (const auto& pos1 : *set1) {
			for (const auto& pos2 : *set2) {
				combi.str("");
				combi << pos1 << "-";
				combi << pos2;
				_rankCombinationSet.emplace_back(combi.str());
			}
		}
		std::swap(set1, set2);
		num++;
	}
	
	/*2. 2CAP-{1FO/2FO}*/
	for (int i = 0; i < CAP_positions.size(); i++) {
		for (int j = 0; j < CAP_positions.size(); j++) {
			if ((CAP_positions[i] == "C4" && CAP_positions[j] == "T3")
				|| (CAP_positions[i] == "T3" && CAP_positions[j] == "C4")) {
				continue;
			}
			
			combi.str("");
			combi << CAP_positions[i] << "-";
			combi << CAP_positions[j];
			CAP_permus2.emplace_back(combi.str());
		}
	}
	//2CAP1FO
	set1 = &CAP_permus2;
	set2 = &FO_positions;
	num = 0;
	while (num < 2) {
		for (const auto& pos1 : *set1) {
			for (const auto& pos2 : *set2) {
				if ((pos1.find("C4") != string::npos && pos2.find("F6") != string::npos)
					|| (pos1.find("F6") != string::npos && pos2.find("C4") != string::npos)) {
					continue;
				}

				combi.str("");
				combi << pos1 << "-";
				combi << pos2;
				_rankCombinationSet.emplace_back(combi.str());
			}
		}
		std::swap(set1, set2);
		num++;
	}
	//2FO...

	/*3. 3CAP1FO*/
	std::vector<string> CAP_permus3;
	set1 = &CAP_permus2;
	set2 = &CAP_positions;
	num = 0;
	while (num < 2) {
		for (const auto& pos1 : *set1) {
			for (const auto& pos2 : *set2) {
				combi.str("");
				combi << pos1 << "-";
				combi << pos2;
				CAP_permus3.emplace_back(combi.str());
			}
		}
		std::swap(set1, set2);
		num++;
	}
	set1 = &CAP_permus3;
	set2 = &FO_positions;
	num = 0;
	while (num < 2) {
		for (const auto& pos1 : *set1) {
			for (const auto& pos2 : *set2) {
				combi.str("");
				combi << pos1 << "-";
				combi << pos2;
				_rankCombinationSet.emplace_back(combi.str());
			}
		}
		std::swap(set1, set2);
		num++;
	}


	#pragma endregion

}

void RollingOpt::setSeqMaps() {
	std::vector<string> positionSet(CAP_positions);
	positionSet.resize(CAP_positions.size() + FO_positions.size());
	std::copy(FO_positions.begin(), FO_positions.end(), positionSet.begin() + CAP_positions.size());
	for (const auto& pos : positionSet) {
		posOrderSeqVec* posv = new posOrderSeqVec;
		posv->position = pos;
		posv->orderSeqIdvec.clear();

		_pos_order_seqs[pos] = posv;
	}
	int numOrder = 4;//号位的数量
	for (int order = 0; order < numOrder; order++) {
		orderPosSeqVec* opsv = new orderPosSeqVec;
		opsv->order = order;
		opsv->posSeqIdvec.clear();

		_order_pos_seqs[order] = opsv;
	}

	const char* pos;
	const char* seq;
	std::vector<string> combi;
	for (int i = 0; i < _rankCombinationSet.size(); i++) {
		seq = _rankCombinationSet[i].c_str();
		combi.clear();
		split(seq, '-', combi);
		for (int order = 0; order < combi.size(); order++) {
			pos = combi[order].c_str();
			_pos_order_seqs[pos]->orderSeqIdvec[order].emplace_back(i);
			
			_order_pos_seqs[order]->posSeqIdvec[pos].emplace_back(i);
		}
	}
}
void RollingOpt::sort_SeqMaps() {
	for (auto& k_v : _pos_order_seqs) {
		for (auto& order_idSet : k_v.second->orderSeqIdvec) {
			std::sort(order_idSet.second.begin(), order_idSet.second.end());
		}
	}

	for (auto& k_v : _order_pos_seqs) {
		for (auto& pos_idSet : k_v.second->posSeqIdvec) {
			std::sort(pos_idSet.second.begin(), pos_idSet.second.end());
		}
	}
}
