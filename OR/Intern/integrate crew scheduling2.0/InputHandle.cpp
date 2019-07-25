#include "InputHandle.h"


void InputHandler::typeTrans(std::vector<void*>& objSet, const string& objName, void* outObjSet) {		
	if (objName == "Flight") {		
		std::vector<Segment*>* ref = (std::vector<Segment*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (Segment*)objSet[i];
		}
	}
	else if (objName == "Base") {
		std::vector<BASE*>* ref = (std::vector<BASE*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (BASE*)objSet[i];
		}
	}
	else if (objName == "Crew") {
		std::vector<CREW*>* ref = (std::vector<CREW*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (CREW*)objSet[i];
		}
	}
	else if (objName == "CrewRank") {
		std::vector<CREW_RANK*>* ref = (std::vector<CREW_RANK*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (CREW_RANK*)objSet[i];
		}
	}
	else if (objName == "CrewBase") {
		std::vector<CREW_BASE*>* ref = (std::vector<CREW_BASE*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (CREW_BASE*)objSet[i];
		}
	}
	else if (objName == "FlightComposition") {
		std::vector<csvActivityComposition*>* ref = (std::vector<csvActivityComposition*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (csvActivityComposition*)objSet[i];
		}
	}
	else if (objName == "Composition") {
		std::vector<csvComposition*>* ref = (std::vector<csvComposition*>*)outObjSet;
		ref->resize(objSet.size());
		for (int i = 0; i < objSet.size(); i++) {
			(*ref)[i] = (csvComposition*)objSet[i];
		}
	}
	else { std::cout << "ERROR: invalid objName " << objName << "\n"; }
}

void InputHandler::matchSegmentAndComposition(std::vector<Segment*>* segSet, 
	std::vector<csvActivityComposition*>* fltCompoSet,
	std::vector<csvComposition*>* compoSet) {
	for (auto& seg : *segSet) {
		for (const auto& flt_compo : *fltCompoSet) {
			if (flt_compo->activityId == seg->getDBId()) {
				seg->fltCompo = flt_compo;				
				for (const auto& compo : *compoSet) {
					if (compo->compId == flt_compo->compId) {
						seg->composition = compo;
						break;
					}
				}
				break;
			}
		}
	}
}
void InputHandler::matchCrewAndRank(std::vector<CREW*>* crewSet, std::vector<CREW_RANK*>* crewrankSet) {
	std::map<string, CREWRankAry*> id_rank;

	for (const auto& crewrank : *crewrankSet) {
		if (id_rank.find(crewrank->idCrew) == id_rank.end()) {
			id_rank[crewrank->idCrew] = new CREWRankAry();
		}
		id_rank[crewrank->idCrew]->emplace_back(crewrank);
	}

	for (auto& crew : *crewSet) {
		crew->rankAry = id_rank[crew->idCrew];
	}
}
void InputHandler::matchCrewAndBase(std::vector<CREW*>* crewSet, std::vector<CREW_BASE*>* crewbaseSet) {
	std::map<string, CrewBaseAry*> id_base;
	for (auto& base : *crewbaseSet) {
		if (id_base.find(base->idCrew) == id_base.end()) {
			id_base[base->idCrew] = new CrewBaseAry();
		}
		id_base[base->idCrew]->emplace_back(base);
	}
	for (auto& crew : *crewSet) {
		crew->baseAry = id_base[crew->idCrew];
	}
}
void InputHandler::sortCrewRank(CREWRankAry* crewrankAry) {
	//对每个crew的rank按最新的时间排序	
	std::sort(crewrankAry->begin(), crewrankAry->end(),
		[](const CREW_RANK *a, const CREW_RANK *b) {return a->effUtc > b->effUtc; });

}
std::vector<CREW*>* InputHandler::getPilotSet(const std::vector<CREW*>& crewSet) {
	std::vector<CREW*>* pilotSet = new std::vector<CREW*>();

	for (const auto& crew : crewSet) {
		if (crew->division == "P") {
			
			for (auto iter = crew->rankAry->begin(); iter != crew->rankAry->end();) {
				if ((*iter)->rank != "CAP" && (*iter)->rank != "FO") {
					iter = crew->rankAry->erase(iter);
				}
				else {
					iter++;
				}
			}

			pilotSet->emplace_back(crew);

		}
	}

	return pilotSet;
}

void InputHandler::getAirportSet(const std::vector<Segment*>& fltSet) {
	for (const auto& flt : fltSet) {
		if (std::find(airportSet->begin(), airportSet->end(), flt->getDepStation()) == airportSet->end()) {
			airportSet->emplace_back(flt->getDepStation());
		}
		else if (std::find(airportSet->begin(), airportSet->end(), flt->getArrStation()) == airportSet->end()) {
			airportSet->emplace_back(flt->getArrStation());
		}
	}
}
std::vector<string>* InputHandler::createSpecialArpSet() {
	std::vector<string>* specialArpSet = new std::vector<string>();
	//srand(0);	
	int index = 0;
	int end = airportSet->size();
	for (int i = 0, num = 5; i < num; i++) {
		index = rand() % end + i;
		specialArpSet->emplace_back((*airportSet)[index]);
	}
	return specialArpSet;
}
