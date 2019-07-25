#pragma once
#include "pch.h"

#include "IntegrateOptimization.h"
#ifndef OUTPUT_H
#define OUTPUT_H

static std::vector<string> scheduleHeader = { "crewID","rank","position", "startDtLoc", "endDtLoc","dutys content<startDt-endDt-<fltId-fltNum>" };
static std::vector<string> crewStatusHeader = { "crewID","rank","position", "totalFlyMin","totalCreditMint" };
	
class Outputer
{
public:
	void receiveCrewSet(std::vector<CREW*>& crewSet);
	//��ʽ��crewID-rank-position
	//dutys:startDt-endDt-<fltId-fltNum-day off-fltId-fltNum>
	void writeSchedule(const std::string& outputSchFile);
	//дcrew�����ƻ����ڵ����幤��ָ��
	//totalFlyMint-totalCreditMint
	void writeCrewStatus(const std::string& outputStatusFile);

private:
	std::vector<CREW*>	_CrewSet;
	
};

#endif // !OUTPUT_H
