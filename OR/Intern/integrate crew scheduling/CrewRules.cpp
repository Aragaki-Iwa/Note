#include "pch.h"
#include "CrewRules.h"

void CrewRules::setHorizonRules(int minTransMin,
	int maxTransMin,
	int minOutRestMin,
	int maxOutRestMin,
	int minFlyMin,
	int maxFlyMin,
	int minDutyMin,
	int maxDutyMin)
{
	horizon_rules->minTransMin = minTransMin;
	horizon_rules->maxTransMin = maxTransMin;
	horizon_rules->minOutRestMin = minOutRestMin;
	horizon_rules->maxOutRestMin = maxOutRestMin;
	horizon_rules->minFlyMin = minFlyMin;
	horizon_rules->maxFlyMin = maxFlyMin;
	horizon_rules->minDutyMin = minDutyMin;
	horizon_rules->maxDutyMin = maxDutyMin;		
}
void CrewRules::setWeekPara(int maxCreditMinutes, int maxWeekFlyMinutes, int minDayOffMinutes, int allowOverCreditMinutes) {
	maxCreditMin = maxCreditMinutes;
	maxWeekFlyMin = maxWeekFlyMinutes;
	minDayOffMin = minDayOffMinutes;
	allowOverCreditMin = allowOverCreditMinutes;
}

void CrewRules::addRankCombination(std::string& rankCombination) {
	rankCombinationSet.insert(rankCombination);
}
