#pragma once
#include <unordered_set>
#ifndef CREW_RULES_H
#define CREW_RULES_H

class Opt_CREW;
using seqIdVec = std::vector<int>; //存放seq在_rankCombinationSet中的id

class PosOrderSeqvec
{
public:
	~PosOrderSeqvec() {
		for (auto& k_v : orderSeqIdvec) {
			k_v.second.clear();
			k_v.second.shrink_to_fit();
		}
		orderSeqIdvec.clear();
	}
	std::string position;
	std::map<int, seqIdVec> orderSeqIdvec;
};
class OrderPosSeqvec
{
public:
	~OrderPosSeqvec() {
		for (auto& k_v : posSeqIdvec) {
			k_v.second.clear();
			k_v.second.shrink_to_fit();
		}
		posSeqIdvec.clear();
	}
	int order;
	std::map<std::string, seqIdVec> posSeqIdvec;
};



static std::vector<std::string> CAP_positions{ "C1","C2", "C4", "T1", "T2", "T3", "K1", "K2", "K3" };
static std::vector<std::string> FO_positions{ "F1", "F2", "F3", "F4", "F5", "F6", "J1", "J2" };

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
	void setWeekPara(int maxCreditMinutes, int maxWeekFlyMinutes, int minDayOffMinutes, int maxDayOffMinutes/*allowOverCreditMinutes*/);
		
	void addRankCombination(const std::string& rankCombination);
	void setSeqMaps();
	void sortSeqMaps();


	bool isFasibleCombination(const std::vector<Opt_CREW*>& crewComb);

	HORIZON_RULES* horizon_rules;
	//最大执勤时长，超过后必须day off
	int maxCreditMin;
	//最大周飞行时间，超过后必须day off
	int maxWeekFlyMin;	
	//day off至少休息时间
	int minDayOffMin;
	//day off至多休息时间
	int maxDayOffMin;
	//允许超过最大执勤时长的量
	//int allowOverCreditMin;

	std::vector<std::string> rankCombinationSet;
	std::map<std::string, PosOrderSeqvec*> _pos_order_seqs;
	std::map<int, OrderPosSeqvec*> _order_pos_seqs;

	// added-20190819
	std::map<std::string, std::vector<std::string>> compo_sequences_map;
};

#endif // !CREW_RULES_H
