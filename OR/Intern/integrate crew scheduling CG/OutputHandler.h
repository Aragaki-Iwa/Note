#pragma once
#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H
#include "..\csvReader\UtilFunc.h"
#include "column_generation.h"
#include <iostream>
#include <sstream>
#include <fstream>

static std::vector<string> scheduleHeader = { "crewID","rank","position", "startDtLoc", "endDtLoc","dutys content<startDt-endDt-<fltId-fltNum>" };
static std::vector<string> crewStatusHeader = { "crewID","rank","position", "accumFltMin","accumCreditMin","totalFlyMin","totalCreditMint" };


class OutputHandler 
{
public:
	//! 记录单天的计划
	//！包括为覆盖的班
	void writeSchedule(const Solution& soln, const SegNodeSet& curDaySegSet, const std::string& schFile);
	//! 记录单天crew的状态
	//! 以乘务组为单位
	void writeCrewStatus(const Solution& soln, const std::string& statusFile);
	//! 记录整个周期每个crew的状态
	void writeCrewStatus(const vector<Opt_CREW*>& crewSet, const char* statusFile);
};

#endif // !OUTPUT_HANDLER_H

