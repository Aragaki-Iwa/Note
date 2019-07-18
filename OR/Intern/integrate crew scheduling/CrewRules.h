#pragma once
#include <unordered_set>
#ifndef CREW_RULES_H
#define CREW_RULES_H


typedef struct HorizontalRules
{
	int minTransMin;
	int maxTransMin;
	int minOutRestMin;
	int maxOutRestMin;
	int minFlyMin;
	int maxFlyMin;
	int minDutyMin;
	int maxDutyMin;

}HORIZON_RULES;
typedef struct VerticalRules
{

};

class CrewRules
{
public:
	CrewRules() {
		horizon_rules = new HORIZON_RULES();
	}
	void setHorizonRules(int minTransMin,
		int maxTransMin,
		int minOutRestMin,
		int maxOutRestMin,
		int minFlyMin,
		int maxFlyMin,
		int minDutyMin,
		int maxDutyMin);
	void setWeekPara(int maxCreditMinutes, int maxWeekFlyMinutes, int minDayOffMinutes, int allowOverCreditMinutes);
	void addRankCombination(std::string& rankCombination);


	HORIZON_RULES* horizon_rules;
	//最大执勤时长，超过后必须day off
	int maxCreditMin;
	//最大周飞行时间，超过后必须day off
	int maxWeekFlyMin;	
	//day off至少休息时间
	int minDayOffMin;
	//允许超过最大执勤时长的量
	int allowOverCreditMin;

	std::unordered_set<std::string> rankCombinationSet;
};

#endif // !CREW_RULES_H
