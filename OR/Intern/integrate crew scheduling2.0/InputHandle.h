#pragma once
#include "pch.h"
#include "..\csvReader\csv\csv_impl.h"
#include "..\CsvReader\Segment_mine.h"
#include "..\csvReader\crewDB_mine.h"
#include "..\csvReader\UtilFunc.h"

#ifndef INPUT_HANDLE_H
#define INPUT_HANDLE_H

class InputHandler
{
public:	
	InputHandler() {
		airportSet = new std::vector<std::string>();
	}
	~InputHandler() {
		airportSet->clear();
		airportSet->shrink_to_fit();
		delete airportSet;
	}
public:
	void typeTrans(std::vector<void*>& objSet, const string& objName, void* outObjSet);

	void matchSegmentAndComposition(std::vector<Segment*>* segSet, 
		std::vector<csvActivityComposition*>* fltCompoSet,
		std::vector<csvComposition*>* compoSet);
	void matchCrewAndRank(std::vector<CREW*>* crewSet, std::vector<CREW_RANK*>* crewrankSet);
	void matchCrewAndBase(std::vector<CREW*>* crewSet, std::vector<CREW_BASE*>* crewbaseSet);
	void sortCrewRank(CREWRankAry* crewrankAry);

	void getAirportSet(const std::vector<Segment*>& fltSet);
	std::vector<std::string>* createSpecialArpSet();
	//ÏÈ¿¼ÂÇ·ÉÐÐÔ±
	std::vector<CREW*>* getPilotSet(const std::vector<CREW*>& crewSet);
public:
	std::vector<std::string>* airportSet;
};

#endif // !INPUT_HANDLE_H
