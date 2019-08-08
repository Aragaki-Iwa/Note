#pragma once
#ifndef SEG_NETWORK_H
#define SEG_NETWORK_H
#include <string>
#include "INetwork.h"

class BASE;
class CrewRules;
class Opt_Segment;

class SegNode;
class SegArc;
class Label;
using SegNodeSet = std::vector<SegNode*>;
using SegArcSet = std::vector<SegArc*>;

class SegNode : public INode
{
public:
	SegNode();
	SegNode(Opt_Segment& optSeg);
	virtual ~SegNode();
	virtual void setNodeType(NodeType type);

	SegArcSet outArcSet;
	SegArcSet inArcSet;

	Opt_Segment* optSegment;
	std::string depStation;
	std::string arvStation;
	//std::string name;
	time_t startDtUtc;
	time_t startDtLoc;
	time_t endDtUtc;
	time_t endDtLoc;
	int taskMin;

	
	//std::vector<Label*> labelSet;
	Label* label;
	
};

class SegArc : public IArc
{
public:
	SegArc(SegNode* startNode, SegNode* endNode);
	virtual ~SegArc();
	virtual void setArcType(ArcType type);
	virtual void setArcLen(int length);

	SegNode* startNode;
	SegNode* endNode;
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
	SegArc* preArc;
};

class SegNetwork : public INetwork
{
public:
	SegNetwork(std::vector<Opt_Segment*>* optSegSet, std::vector<BASE*>* baseSet, CrewRules* rules);
		
	virtual ~SegNetwork();

	virtual void createNetwork();

	//resource and sink node updated every day 
	void updateResourceAndSink();
	//vitual arcs updated every day
	void updateVirtualArcs(SegNodeSet& startNodeSet);
	
	
	SegNode* resource;
	SegNode* sink;

	SegNodeSet nodeSet;
	SegArcSet arcSet;

private:
	std::vector<Opt_Segment*>* _optSegSet;
	std::vector<BASE*>* _baseSet;
	CrewRules* _rules;
	std::vector<SegNode*> _base_nodeset;

	void setOptSegNodeSet(/*const std::vector<Opt_Segment*>& optSegSet, std::vector<BASE*>& baseSet*/);

	void createOptSegNodes(/*const std::vector<Opt_Segment*>& optSegSet*/);
	void createDeadheadNodes();
	//! sort by startTime ascending
	void sortNodeSet();
	//! create segnode for base
	//! store these node in _base_nodeset
	void createBaseNodes();
	
	//create only once
	void createOptSegArcs(/*const CrewRules& rules*/);
	bool isSameFleetConnectble(const Opt_Segment& seg1, const Opt_Segment& seg2);
	bool isDiffFleetConnectble(const Opt_Segment& seg1, const Opt_Segment& seg2, const int minTransMint);	
	void createBaseArcs();
	//! add extra long arcs for node whose in/out degree is little
	void addLongArcs();
	//! delete segnode whose indegree or outdegree equals zero
	void removeIsolatedNode();

};

#endif // !SEG_NETWORK_H