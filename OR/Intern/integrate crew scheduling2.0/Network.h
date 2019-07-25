#pragma once
#ifndef NETWORK_H
#define NETWORK_H
#include "pch.h"
#include "..\CsvReader\Segment_mine.h"
#include "..\csvReader\crewDB_mine.h"
#include "..\csvReader\UtilFunc.h"
#include "..\csvReader\csv\csv_impl.h"
#include "CrewRules.h"

typedef enum NodeType
{
	resource,
	sink,
	baseO,
	baseD,
	seg,
	dhd
};
typedef enum ArcType
{
	resourceOut,
	sinkIn,
	baseOut,
	baseIn,
	segConnect,
	rest
	//deadhead
};
class Node;
class Arc;
using NodeSet = std::vector<Node*>;
using ArcSet = std::vector<Arc*>;
using BASESet = std::vector<BASE*>;
class Label;
class Node
{
public:
	Node() { 
		segment = NULL; 
		outArcSet = new ArcSet();
		inArcSet = new ArcSet();
	}
	~Node()
	{
		if (segment != NULL) { delete segment; segment = NULL; }
		delete outArcSet; outArcSet = NULL;
		delete inArcSet; inArcSet = NULL;
		labelSet.clear();
		labelSet.shrink_to_fit();
	}
	Segment* segment;
	string depStation;
	string arvStation;
	std::string name;
	time_t startDtUtc;
	time_t startDtLoc;
	time_t endDtUtc;
	time_t endDtLoc;
	int taskMin;
	NodeType nodeType;	
	ArcSet* outArcSet;
	ArcSet* inArcSet;
	//find path
	bool visited;
	std::vector<Label*> labelSet;
	bool becameStartNode;
	//roster
	bool assigned;//被分配或是被取消了
};

class Arc
{
public:
	Node* startNode;
	Node* endNode;
	int len;
	ArcType arcType;

};

class Label
{
public:
	Label() {
		accumuFlyMin = 0;
		accumuWorkMin = 0;
		accumuCost = 0;
		preLabel = NULL;
		preArc = NULL;
	}
	int accumuFlyMin;
	int accumuWorkMin;
	int accumuCost;
	Label* preLabel;
	Arc* preArc;
};

class Network
{
public:
	Network()
	{
		nodeSet = new NodeSet();
		arcSet = new ArcSet();
	}
	~Network()
	{
		for (auto& node : *nodeSet)
		{
			delete node;
		}
		delete nodeSet;
		for (auto& arc : *arcSet)
		{
			delete arc;
		}
		delete arcSet;
	}
	NodeSet* nodeSet;
	ArcSet* arcSet;
	BASESet* baseSet;

	void createNetwork(const std::vector<Segment*>& segSet, BASESet& baseSet, const CrewRules& rules);
	
private:
	void setNodeSet(const std::vector<Segment*>& segSet,  BASESet& baseSet);
	/*基地与航班节点之间的弧是这样的：基地-航班，默认基地与该航班是同一天。则出乘弧的出发时间是航班的出发时间年月日时分秒，*/
	std::vector<Segment*>* getEarliestAndLatestFlt(const std::vector<Segment*>& segSet);
	void initResourceAndSink(const std::vector<Segment*>& segSet, Node* resource, Node* sink);
	void createBaseNodes(/*std::vector<Node*>& baseNodeSet,*/ const BASESet& baseSet, const Node& resource, const Node& sink);
	void createSegmentNodes(/*std::vector<Node*>* segNodeSet, */const std::vector<Segment*>& segSet);
	void createDeadheadNodes(/*std::vector<Node*>* segNodeSet*/);

	void createVirtualArcs();
	void createBaseArcs();
	void createSegArcs(const HORIZON_RULES& horizonRules);
	bool checkSameTailConnect(Segment& seg1, Segment& seg2);
	bool checkDiffTailConnect(Segment& seg1, Segment& seg2, const int minTransMint);
	void addRestArcs(const HORIZON_RULES& horizonRules);

	void removeIsolatedNodes();
	void renewResourceTime();

	/*7-11*/
	//第一天出发的seg与resource相连，类似于baseOutArc
	//不建立resource与base_o之间的弧
	void createFirstDayArcs();
};
#endif // !NETWORK_H