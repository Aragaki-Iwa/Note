// RO.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "InputHandle.h"
#include "Network.h"
#include "FindDutySet.h"
#include "IntegrateOptimization.h"
#include "Output.h"
#include "SummeryTool_H.h"

using namespace Summery;

int main()
{		
	//clock_t begin;
	//clock_t end;
	//double dur = 0;
	//begin = clock();
	//int x = 1230;
	//std::string s = "cap";
	//for (int i = 0; i < 1000000000; i++) {
	//	/*if (x == 1230) {*/
	//	if (s == "cap") {
	//		int y = 0;
	//		y = y + 1;
	//	}
	//}
	//end = clock();
	//dur = (double)(end - begin);
	//printf("Use Time:%f\n", (dur / CLOCKS_PER_SEC));
	 
	struct cmpCreditMin
	{
		bool operator()(const CREW* c1, const CREW* c2)const {
			return c1->workStatus->accumuCreditMin < c2->workStatus->accumuCreditMin;
		}
	};
	CREW* c1 = new CREW();
	CREW* c2 = new CREW();
	c1->workStatus->accumuCreditMin = 10;
	c2->workStatus->accumuCreditMin = 20;
	std::map<CREW*, int, cmpCreditMin> test;
	test[c1] = c1->workStatus->accumuCreditMin;
	test[c2] = c2->workStatus->accumuCreditMin;
	c1->workStatus->accumuCreditMin = 30;
	test.key_comp();


	crewCsvReader crew_csv_reader;	
	std::vector<string> objNameSet = { "Flight","Base","Crew","CrewRank","CrewBase", "FlightComposition", "Composition" };	
	crew_csv_reader.readMutiTableCsv("ro_input.txt");
	auto allTable = crew_csv_reader.datas;

	CrewRules rules;
	rules.setHorizonRules(15, 600, 100, 720, 80, 400, 80, 600);
	//rules.setDayOffPara(2000, 2160, 300);
	rules.setWeekPara(7200, 1200, 2160, 120);
	
	StopWatch sw;

	RollingOpt opt;
	opt.inputData(allTable, objNameSet);
	opt.init();
	//construct test
	opt.randomSet_specialAirport();
	opt.randomSet_crewSkills();
	
	sw.Start();
	opt.set_rankCombination();	
	sw.Stop();
	std::cout << "set rankCombinations spend time: " << sw.Elapsed_s() << " s\n";

	sw.Restart();
	opt.setSeqMaps();
	sw.Stop();
	std::cout << "set rankMaps spend time: " << sw.Elapsed_s() << " s\n";
	sw.Restart();
	opt.sort_SeqMaps();
	sw.Stop();
	std::cout << "set sort rankMaps spend time: " << sw.Elapsed_s() << " s\n";
	
 	opt.setRules(rules);
	//over construct test

	opt.optimize();

	Outputer outputer;
	string sch_file("Schedule_output.txt");
	string status_file("CrewStatus_ouput.txt");
	auto crewSet = opt.getCrewSet();
	outputer.receiveCrewSet(crewSet);

	outputer.writeSchedule(sch_file);
	outputer.writeCrewStatus(status_file);



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
