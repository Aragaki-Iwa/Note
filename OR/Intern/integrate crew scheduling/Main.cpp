// RO.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "InputHandle.h"
#include "Network.h"
#include "FindDutySet.h"
#include "IntegrateOptimization.h"

void randomSet_specialAirports(std::vector<DBAirport*>& specialAirportSet) {
	int num = specialAirportSet.size() - 4;
	for (auto iter = specialAirportSet.begin(); iter != specialAirportSet.end();) {
		if (rand() % 2 == 1) {
			iter = specialAirportSet.erase(iter);
			--num;
		}
		else {
			iter++;
		}
		if (num <= 0) {
			break;
		}
	}  

	if (specialAirportSet.size() > 4) {
		specialAirportSet.erase(specialAirportSet.begin() + 4, specialAirportSet.end());
	}
}


int main()
{		

	crewCsvReader crew_csv_reader;	
	std::vector<string> objNameSet = { "Flight","Base","Crew","CrewRank","CrewBase" };	
	crew_csv_reader.readMutiTableCsv("ro_input.txt");
	auto allTable = crew_csv_reader.datas;

	/*std::vector<Segment*> segSet;
	std::vector<BASE*> baseSet;
	std::vector<CREW*> crewSet;
	std::vector<CREW_RANK*> crew_rankSet;
	std::vector<CREW_BASE*> crew_baseSet;

	InputHandler inputHandler;
	inputHandler.typeTrans(allTable["Flight"], "Flight", &segSet);
	inputHandler.typeTrans(allTable["Base"], "Base", &baseSet);
	inputHandler.typeTrans(allTable["Crew"], "Crew", &crewSet);
	inputHandler.typeTrans(allTable["CrewRank"], "CrewRank", &crew_rankSet);
	inputHandler.typeTrans(allTable["CrewBase"], "CrewBase", &crew_baseSet);*/

	CrewRules rules;
	rules.setHorizonRules(15, 600, 100, 720, 80, 400, 80, 600);
	//rules.setDayOffPara(2000, 2160, 300);
	rules.setWeekPara(7200, 2400, 2160, 120);
	
	//Network net;
	//net.createNetwork(segSet, baseSet, rules);

	//std::vector<Node*> startNodeSet;
	//PathFinder path_finder;
	//path_finder.findPathSet(&net, startNodeSet, rules);
	//std::vector<Path*> pathSet = *path_finder.getPathSet();
	//std::sort(pathSet.begin(), pathSet.end(),
	//	[](const Path *a, const Path *b) { 
	//	return a->route.front()->segment->getStartTimeLocSch() < b->route.front()->segment->getStartTimeLocSch(); } );
	//
	//std::cout << "/*********************************************/\n";
	//for (auto& path : pathSet) {
	//	//std::cout << path->route.front()->segment->getDate() << "\n";
	//	for (auto& node : path->route) {
	//		node->assigned = true;
	//	}
	//}
	//std::vector<Node*> un_assignedNodes;
	//time_t last = pathSet.back()->route.front()->startDtUtc;
	//for (int i = 2 + 2 * baseSet.size(); i < net.nodeSet->size(); i++) {
	//	auto node = (*net.nodeSet)[i];
	//	if (node->endDtUtc < last && node->assigned == false && node->nodeType==NodeType::seg) {
	//		un_assignedNodes.emplace_back(node);
	//		std::cout  << "depArp: " << node->segment->getDepStation() << " fltNum:" << node->segment->getFlightNumber() << " fltDt:" << node->segment->getDate() << "\n";
	//	}
	//}

	RollingOpt opt;
	opt.inputData(allTable, objNameSet);	
	opt.init();
	//construct test
	opt.randomSet_specialAirport();
	opt.randomSet_crewSkills();
	opt.set_rankCombination();
	opt.setRules(rules);
	//over construct test

	opt.optimize();

 	system("pause");
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
