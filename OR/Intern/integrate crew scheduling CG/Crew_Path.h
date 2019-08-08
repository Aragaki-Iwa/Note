#pragma once
#ifndef CREW_PATH_H
#define CREW_PATH_H
#include "pch.h"
#include "IPath.h"

class Opt_CREW;
class CrewNode;

struct CrewGroupPrice
{
	CrewGroupPrice() {
		fly_balance_price = 0;
		_total_price = 0;
	}
	
	void calTotalPrice() {		
		_total_price = fly_balance_price; //+..
	}
	double getTotalPrice() const {
		return _total_price;
	}
		
	double fly_balance_price;
	
private:
	double _total_price;

};

class CrewGroup :public IPath
{
public:
	virtual ~CrewGroup();

	virtual double getCost() const;
	
	void computeCost();
	std::vector<CrewNode*>& getNodeSequence() { return _crewNodeSequence; }
	void setCrewGroup();	
	std::vector<Opt_CREW*>& getCrewGroup() { return _crewGroup; }

	void setCrewIndexSet(/*const std::vector<Opt_CREW*>& optCrewSet*/);
	std::vector<int> optcrew_id_set;

	/*8-6-2019*/
	void setBasicProperties();
	std::map<std::string, int> spetialCredentials; //只要有一个crew没有特殊资质，则整个group都没有	
	int max_fly_mint;
	int max_credit_mint;
	time_t endDtLoc;


private:
	int _nbCrewNodes;
	std::vector<CrewNode*> _crewNodeSequence;
	CrewGroupPrice _crewsPrice;
	std::vector<Opt_CREW*> _crewGroup;
};

#endif // !CREW_PATH_H