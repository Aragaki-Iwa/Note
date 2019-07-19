#include "Output.h"

void Outputer::receiveCrewSet(std::vector<CREW*>& crewSet) {
	_CrewSet = crewSet;
}
void Outputer::writeSchedule(const std::string& outputSchFile) {
	ofstream outf;
	outf.open(outputSchFile, std::ios::out);

	for (int i = 0; i < scheduleHeader.size() - 1; i++) {
		outf << scheduleHeader[i] << "^";
	}
	outf << scheduleHeader.back() << "\n";

	for (const auto& crew : _CrewSet) {
		auto crew_dutySet = crew->workStatus->CreditedDutySet;
		outf << "Crew: " << crew->idCrew << " "
			 << "Rank: " << crew->rankAry->front()->rank << " "
			 << "Position: " << crew->rankAry->front()->position << "\n";
		
		
		if (crew_dutySet.size() == 0) {
			continue;
		}

		for (const auto& duty : crew_dutySet) {
			outf << "    ----duty:";
			outf << "                              startDtLoc: " << utcToUtcString(duty->startDtLoc) << " "
				<< "endDtLoc: " << utcToUtcString(duty->endDtLoc) << " "
				<< "startStation: " << duty->startStation << " "
				<< "endStation: " << duty->endStation << "\n";

			
			for (const auto& node : duty->route) {
				outf << "        segment: ";
				outf << "id: " << node->segment->getDBId() << " "
					<< "fltNum: " << node->segment->getFlightNumber() << " "
					<< "depDtLoc: " << utcToUtcString(node->segment->getStartTimeLocSch()) << " "
					<< "arvDtLoc: " << utcToUtcString(node->segment->getEndTimeLocSch()) << " "
					<< "depArp: " << node->segment->getDepStation() << " "
					<< "arvArp: " << node->segment->getArrStation() << "\n";
			}			
		}

		outf << "\n";
	}
	outf.close();
}

void Outputer::writeCrewStatus(const std::string& outputStatusFile) {
	ofstream outf;
	outf.open(outputStatusFile, std::ios::out);

	for (int i = 0; i < crewStatusHeader.size() - 1; i++) {
		outf << crewStatusHeader[i] << "^";
	}
	outf << crewStatusHeader.back() << "\n";

	for (const auto& crew : _CrewSet) {
		auto crew_status = crew->workStatus;
		auto crew_rank = crew->rankAry->front();

		outf << crew->idCrew << "^"
			<< crew_rank->rank << "^"
			<< crew_rank->position << "^ | ";
		outf << to_string(crew_status->totalFlyMint) << "^"
			<< to_string(crew_status->totalCreditMint) << "\n";
	}	

	outf.close();
}