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
	//���ִ��ʱ�������������day off
	int maxCreditMin;
	//����ܷ���ʱ�䣬���������day off
	int maxWeekFlyMin;	
	//day off������Ϣʱ��
	int minDayOffMin;
	//���������ִ��ʱ������
	int allowOverCreditMin;

	std::unordered_set<std::string> rankCombinationSet;
};

#endif // !CREW_RULES_H
