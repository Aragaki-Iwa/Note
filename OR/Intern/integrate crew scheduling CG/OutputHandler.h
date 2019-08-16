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
	//! ��¼����ļƻ�
	//������Ϊ���ǵİ�
	void writeSchedule(const Solution& soln, const SegNodeSet& curDaySegSet, const std::string& schFile);
	//! ��¼����crew��״̬
	//! �Գ�����Ϊ��λ
	void writeCrewStatus(const Solution& soln, const std::string& statusFile);
	//! ��¼��������ÿ��crew��״̬
	void writeCrewStatus(const vector<Opt_CREW*>& crewSet, const char* statusFile);
};

#endif // !OUTPUT_HANDLER_H

