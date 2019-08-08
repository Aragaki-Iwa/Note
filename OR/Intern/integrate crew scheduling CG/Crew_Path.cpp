#include "Crew_Path.h"
#include "Crew_Network.h"
#include "csvClassesTransOptimizable.h"

CrewGroup::~CrewGroup() {
	_crewNodeSequence.clear();
}

double CrewGroup::getCost() const {
	return _crewsPrice.getTotalPrice();
}
void CrewGroup::computeCost() {
	for (const auto& node : _crewNodeSequence) {
		_crewsPrice.fly_balance_price += node->price;
	}	
	_crewsPrice.calTotalPrice();
}
void CrewGroup::setCrewGroup() {
	for (const auto& node : _crewNodeSequence) {
		_crewGroup.emplace_back(node->optCrew);
	}
}

void CrewGroup::setCrewIndexSet(/*const std::vector<Opt_CREW*>& optCrewSet*/) {
	/*auto begin = optCrewSet.begin();
	auto end = optCrewSet.end();
	for (const auto& crewnode : _crewNodeSequence) {
		auto pos = std::find(begin, end, crewnode->optCrew);
		int index = pos - begin;
		optcrew_id_set.emplace_back(index);
	}*/
	for (const auto& crewnode : _crewNodeSequence) {
		optcrew_id_set.emplace_back(crewnode->optCrew->getIndex());
	}

}
void CrewGroup::setBasicProperties() {	
	auto first_status = _crewNodeSequence.front()->optCrew->workStatus;		
	max_fly_mint = first_status->accumuFlyMin;
	max_credit_mint =first_status->accumuCreditMin;	
	endDtLoc = first_status->endDtLoc;
	std::string skill;
	spetialCredentials[skill] = 1;
	
	for (const auto& crewnode : _crewNodeSequence) {	
		spetialCredentials[skill] *= crewnode->optCrew->getSkillSet()[skill];
		
		auto temp_status = crewnode->optCrew->workStatus;
		
		time_t temp_end_loc = temp_status->endDtLoc;
		endDtLoc = temp_end_loc > endDtLoc ? temp_end_loc : endDtLoc;
		
		int temp_fly_mint = temp_status->accumuFlyMin;
		int temp_credit_mint = temp_status->accumuCreditMin;
		max_fly_mint = temp_fly_mint > max_fly_mint ? temp_fly_mint : max_fly_mint;
		max_credit_mint = temp_credit_mint > max_credit_mint ? temp_credit_mint : max_credit_mint;
	}
}
