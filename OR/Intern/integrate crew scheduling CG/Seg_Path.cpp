#include "Seg_Path.h"
#include "..\csvReader\csv\csv_impl.h"
#include "..\csvReader\crewDB_mine.h"
#include "csvClassesTransOptimizable.h"
#include "Seg_Network.h"
#include "Crew_Path.h"

//SegPath::SegPath(Penalty& penaltySetting) { 
//	_penalty_setting = &penaltySetting; 
//}
SegPath::SegPath() {	
}
SegPath::~SegPath() {
	startNode = NULL;
	endNode = NULL;

	_segNodeSequence.clear();
	//_crewGroup.clear();
}

double SegPath::getCost() const {
	return _segPathCost.getTotalCost();
}

void SegPath::init(const Penalty& penaltySetting) {
	_penalty_setting = &penaltySetting;
	_nbSegNodes = _segNodeSequence.size(); 
	startNode = _segNodeSequence.front();
	csvComposition* duty_compo = startNode->optSegment->getCompositon();
	int dhd_count = startNode->nodeType == NodeType::dhd ? 1 : 0;
	for (size_t i = 1; i < _segNodeSequence.size(); i++) {		
		duty_compo = duty_compo->hirarchy > _segNodeSequence[i]->optSegment->getCompositon()->hirarchy
			? duty_compo : _segNodeSequence[i]->optSegment->getCompositon();		

		// TODO:计算特殊资质要求数量
	}
	_path_composition = duty_compo;
	_path_compoMode = &compoModeMap[_path_composition->nameDesc];
	
	endNode = _segNodeSequence.back();
	Label* end_label = endNode->label;
	if (_segNodeSequence.size() > 1) {
		dhd_count = endNode->nodeType == NodeType::dhd ? dhd_count + 1 : dhd_count;
	}	
	
	std::vector<costTuple> cost_tuples = { costTuple(end_label->accumuFlyMin, _penalty_setting->penalty_flt_time) 
										 , costTuple(dhd_count, _penalty_setting->penalty_flt_time) 
										 , costTuple(0, _penalty_setting->penalty_special_credential) };
	computeCost(cost_tuples);

}
void SegPath::computeCost(std::vector<costTuple>& costTuples) {
	//一些cost如fly_time， credit_time, dhd_cost可在搜索路时就求出来了
	_segPathCost.fly_time_costtuple = costTuples[0];
	_segPathCost.dhd_costtuple = costTuples[1];
	_segPathCost.special_credential_costtuple = costTuples[2];
	
	_segPathCost.calTotalCost();
}

void SegPath::setSegIndexSet(const std::vector<Opt_Segment*>& curDayOptSegSet) {
	auto begin = curDayOptSegSet.begin();
	auto end = curDayOptSegSet.end();
	for (const auto& segnode : _segNodeSequence) {
		if (segnode->nodeType == NodeType::dhd) {
			continue;
		}
		auto pos = std::find(begin, end, segnode->optSegment);
		int index = pos - begin;
		optseg_id_sequence.emplace_back(index);
	}
	

}
