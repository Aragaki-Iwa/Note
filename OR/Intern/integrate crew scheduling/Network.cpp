//#include "pch.h"
#include "Network.h"


void Network::createNetwork(const std::vector<Segment*>& segSet, BASESet& baseSet, const CrewRules& rules)
{				
	this->baseSet = &baseSet;
	setNodeSet(segSet, baseSet);
	std::cout << "create nodeSet finished, total number = " << nodeSet->size() << "\n";
	createVirtualArcs();
	std::cout << "create virtual arcs finished, total  arcSet number = " << arcSet->size() << "\n";	
	createBaseArcs();
	std::cout << "create base arcs finished, total arcSet number = " << arcSet->size() << "\n";
	createSegArcs(*rules.horizon_rules);
	std::cout << "create seg arcs finished, total arcSet number = " << arcSet->size() << "\n";
	addRestArcs(*rules.horizon_rules);
	std::cout << "add rest arcs finished, total number = " << arcSet->size() << "\n";
	removeIsolatedNodes();
	std::cout << "remove isolated nodes finished, total nodeSet number = " << nodeSet->size() << "\n";


	renewResourceTime();
	/*7-11*/	
	//createFirstDayArcs();
	std::cout << "create first day arcs finished, total  arcSet number = " << arcSet->size() << "\n";
	/*7-11*/
}

void Network::setNodeSet(const std::vector<Segment*>& segSet,  BASESet& baseSet)
{	
	this->baseSet = &baseSet;
	
	Node* resource = new Node();
	Node* sink = new Node();
	initResourceAndSink(segSet, resource, sink);
	this->nodeSet->emplace_back(resource);
	this->nodeSet->emplace_back(sink);

	//std::vector<Node*> baseNodeSet;
	createBaseNodes(/*baseNodeSet,*/ baseSet, *resource, *sink);
	createSegmentNodes(segSet);
	createDeadheadNodes(/*baseSet.size()*/);

	std::vector<Node*> temp_segNodes(nodeSet->begin() + 2 + 2 * baseSet.size(), nodeSet->end());
	nodeSet->erase(nodeSet->begin() + 2 + 2 * baseSet.size(), nodeSet->end());

	std::sort(temp_segNodes.begin(), temp_segNodes.end(),
		[](const Node* a, Node* b) {return a->startDtUtc < b->startDtUtc; });
	
	nodeSet->insert(nodeSet->end(), temp_segNodes.begin(), temp_segNodes.end());

}
std::vector<Segment*>* Network::getEarliestAndLatestFlt(const std::vector<Segment*>& segSet) 
{
	int index_earliest_depUtc = 0;
	int index_latest_arvUtc = 0;
	time_t earliest_depUtc = segSet[0]->getStartTimeUtcSch();	
	time_t latest_arvUtc = segSet[0]->getEndTimeUtcSch();

	Segment* begin = 0;
	for (int i = 0; i != segSet.size(); i++)
	{
		begin = segSet[i];
		if (earliest_depUtc > begin->getStartTimeUtcSch())
		{
			earliest_depUtc = begin->getStartTimeUtcSch();
			index_earliest_depUtc = i;
		}
		if (latest_arvUtc < begin->getEndTimeUtcSch())
		{
			latest_arvUtc = begin->getEndTimeUtcSch();
			index_latest_arvUtc = i;
		}
	}
	std::vector<Segment*>* earliest_latest_flt = new  std::vector<Segment*>();
	earliest_latest_flt->emplace_back(segSet[index_earliest_depUtc]);
	earliest_latest_flt->emplace_back(segSet[index_latest_arvUtc]);
	return earliest_latest_flt;
}
void Network::initResourceAndSink(const std::vector<Segment*>& segSet, Node* resource, Node* sink)
{
	auto vec = getEarliestAndLatestFlt(segSet);
	Segment* first_dep_flt = vec->at(0);
	Segment* last_arv_flt = vec->at(1);
	resource->startDtUtc = first_dep_flt->getStartTimeUtcSch();
	resource->startDtLoc = first_dep_flt->getStartTimeLocSch();
	resource->endDtUtc = resource->startDtUtc;
	resource->endDtLoc = resource->startDtLoc;
	resource->taskMin = 0;
	resource->depStation = "resource";
	resource->arvStation = resource->depStation;
	resource->name = "resource";
	resource->nodeType = NodeType::resource;
	resource->visited = false;

	sink->startDtUtc = last_arv_flt->getEndTimeUtcSch();
	sink->startDtLoc = last_arv_flt->getEndTimeLocSch();
	sink->endDtUtc = sink->startDtUtc;
	sink->endDtLoc = sink->startDtLoc;
	sink->taskMin = 0;
	sink->depStation = "sink";
	sink->arvStation = sink->depStation;
	sink->name = "sink";
	sink->nodeType = NodeType::sink;
	sink->visited = false;
	delete vec;
}
void Network::createBaseNodes(/*std::vector<Node*>& baseNodeSet, */const BASESet& baseSet, const Node& resource, const Node& sink)
{		
	for (int i = 0; i < baseSet.size(); i++)
	{
		Node* base_o = new Node();
		Node* base_d = new Node();						
		
		base_o->startDtUtc = resource.startDtUtc;
		base_o->startDtLoc = resource.startDtLoc;
		base_o->endDtUtc = base_o->startDtUtc;
		base_o->endDtLoc = base_o->startDtLoc;
		base_o->taskMin = 0;
		base_o->depStation = baseSet[i]->base;
		base_o->arvStation = base_o->depStation;
		base_o->name = baseSet[i]->base;		
		base_o->name += "^O";
		base_o->nodeType = NodeType::baseO;
		base_o->visited = false;

		base_d->startDtUtc = sink.startDtUtc;
		base_d->startDtLoc = sink.startDtLoc;
		base_d->endDtUtc = base_d->startDtUtc;
		base_d->endDtLoc = base_d->startDtLoc;
		base_d->taskMin = 0;
		base_d->depStation = baseSet[i]->base;
		base_d->arvStation = base_d->depStation;
		base_d->name = baseSet[i]->base;
		base_d->name += "^D";
		base_d->nodeType = NodeType::baseD;
		base_d->visited = false;

		this->nodeSet->emplace_back(base_o);
		this->nodeSet->emplace_back(base_d);
	}

}
void Network::createSegmentNodes(/*std::vector<Node*>* segNodeSet,*/ const std::vector<Segment*>& segSet)
{
	for (const auto& seg : segSet)
	{
		Node* node = new Node();
		node->segment = seg;
		node->startDtUtc = seg->getStartTimeUtcSch();
		node->startDtLoc = seg->getStartTimeLocSch();
		node->endDtUtc = seg->getEndTimeUtcSch();
		node->endDtLoc = seg->getEndTimeLocSch();
		node->taskMin = seg->getBlkMinutes();
		node->depStation = seg->getDepStation();
		node->arvStation = seg->getArrStation();
		node->nodeType = NodeType::seg;
		node->visited = false;

		this->nodeSet->emplace_back(node);
	}
}
void Network::createDeadheadNodes(/*std::vector<Node*>* segNodeSet*/)
{
	Node* segNode;
	size_t len = this->nodeSet->size();
	for (size_t i = 2 + 2 * this->baseSet->size(); i < len; i++)
	{
		segNode = (*this->nodeSet)[i];
		Node* dhd_node = new Node();
		dhd_node->segment = segNode->segment;
		dhd_node->depStation = segNode->depStation;
		dhd_node->arvStation = segNode->arvStation;
		dhd_node->startDtUtc = segNode->startDtUtc;
		dhd_node->startDtLoc = segNode->startDtLoc;
		dhd_node->endDtUtc = segNode->endDtUtc;
		dhd_node->endDtLoc = segNode->endDtLoc;
		dhd_node->taskMin = segNode->taskMin;
		dhd_node->nodeType = NodeType::dhd;

		this->nodeSet->emplace_back(dhd_node);
	}
}

void Network::createVirtualArcs()
{
	Node* resource = (*nodeSet)[0];
	Node* sink = (*nodeSet)[1];
	Node* baseNode = NULL;
	for (size_t i = 2, len = 2 + 2 * baseSet->size(); i < len; i++)
	{
		baseNode = (*nodeSet)[i++];

		Arc* out_arc = new Arc();
		out_arc->startNode = resource;
		out_arc->endNode = baseNode;		
		out_arc->len = 0;
		out_arc->arcType = ArcType::resourceOut;
		arcSet->emplace_back(out_arc);
		resource->outArcSet->emplace_back(out_arc);
		baseNode->inArcSet->emplace_back(out_arc);

		baseNode = (*nodeSet)[i];

		Arc* in_arc = new Arc();
		in_arc->startNode = baseNode;
		in_arc->endNode = sink;
		/*in_arc->startDtUtc = (*begin)->arvDtUtc;
		in_arc->startDtLoc = (*begin)->arvDtLoc;
		in_arc->endDtUtc = sink->depDtUtc;
		in_arc->endDtLoc = sink->depDtLoc;*/
		in_arc->len = 0;
		in_arc->arcType = ArcType::sinkIn;
		arcSet->emplace_back(in_arc);
		baseNode->outArcSet->emplace_back(in_arc);
		sink->inArcSet->emplace_back(in_arc);
	}
}
void Network::createBaseArcs()
{
	int base_begin = 2, base_end = 2 + 2 * baseSet->size();
	Node *base_node, *seg_node;

	///*try 7-11-2019*/
	//time_t firstDay = getStartTimeOfDay((*nodeSet)[2]->startDtLoc);
	///*try 7-11-2019*/
	for (int i = base_begin; i < base_end; i++)
	{
		base_node = (*nodeSet)[i];
		for (int j = base_end; j < nodeSet->size(); j++)
		{
			seg_node = (*nodeSet)[j];
			///*try 7-11-2019*/
			//time_t segInDay = getStartTimeOfDay(seg_node->startDtLoc);
			///*try 7-11-2019*/
			if (base_node->arvStation == seg_node->depStation
				&& base_node->nodeType == NodeType::baseO
				/*&& segInDay == firstDay*/)
			{
				Arc* out_arc = new Arc();
				out_arc->startNode = base_node;
				out_arc->endNode = seg_node;
				out_arc->len = (seg_node->startDtLoc - getStartTimeOfDay(seg_node->startDtLoc)) / 60;
				out_arc->arcType = ArcType::baseOut;
				arcSet->emplace_back(out_arc);
				base_node->outArcSet->emplace_back(out_arc);
				seg_node->inArcSet->emplace_back(out_arc);
			}
			else if (base_node->depStation == seg_node->arvStation
				&& base_node->nodeType == NodeType::baseD)
			{
				Arc* in_arc = new Arc();
				in_arc->startNode = seg_node;
				in_arc->endNode = base_node;
				in_arc->len = (getStartTimeOfDay(seg_node->endDtLoc + 24 * 3600) - 1 - seg_node->endDtLoc) / 60;
				in_arc->arcType = ArcType::baseIn;
				arcSet->emplace_back(in_arc);
				base_node->inArcSet->emplace_back(in_arc);
				seg_node->outArcSet->emplace_back(in_arc);
			}
		}
	}
}
void Network::createSegArcs(const HORIZON_RULES& horizonRules)
{
	int len = 0;
	Node *start_node, *end_node;
	int begin = 2 + 2 * baseSet->size(), end = nodeSet->size();
	for (int i = begin; i < end; i++) {
		start_node = (*nodeSet)[i];
		for (int j = begin; j < end; j++) {
			end_node = (*nodeSet)[j];
			len = (end_node->startDtUtc - start_node->endDtUtc) / 60;
			if (start_node->arvStation == end_node->depStation
				&& len > 0
				&& len <= horizonRules.maxTransMin
				&& (checkSameTailConnect(*start_node->segment,*end_node->segment)
					||checkDiffTailConnect(*start_node->segment, *end_node->segment, horizonRules.minTransMin))) {
				Arc* seg_arc = new Arc();
				seg_arc->startNode = start_node;
				seg_arc->endNode = end_node;				
				seg_arc->len = len;
				seg_arc->arcType = ArcType::segConnect;
				arcSet->emplace_back(seg_arc);
				start_node->outArcSet->emplace_back(seg_arc);
				end_node->inArcSet->emplace_back(seg_arc);
			}
		}
	}
}
bool Network::checkSameTailConnect(Segment& seg1, Segment& seg2)
{
	return seg1.getTailNum() == seg2.getTailNum();
}
bool Network::checkDiffTailConnect(Segment& seg1, Segment& seg2, const int minTransMint)
{
	return (seg1.getTailNum() != seg2.getTailNum()) && (seg2.getEndTimeUtcSch() - seg1.getStartTimeUtcSch()>= minTransMint * 60);
}
void Network::addRestArcs(const HORIZON_RULES& horizonRules)
{
	std::vector<Node*> copyNodeSet(*nodeSet);
	//copyNodeSet.resize(nodeSet->size());
	//可以先按时间（或只需到日期层面）排序，然后对出/入度少的点试图用后/前一天的点与其连接跨夜弧
	std::sort(copyNodeSet.begin(), copyNodeSet.end(),
		[](const Node *a, const Node *b) { return a->startDtUtc < b->startDtUtc; });
	Node *cur, *next, *pre;
	int len = 0;
	int begin = 2 + 2 * baseSet->size(), end = nodeSet->size();
	for (int i = begin; i < end; i++) {
		cur = (*nodeSet)[i];
		if (cur->outArcSet->size() <= 2) {
			for (int j = i; j < end; j++) {
				next = (*nodeSet)[j];

				len = (next->startDtUtc - cur->endDtUtc) / 60;
				//rules.maxTransMint < intervalMin是为了保证不和之前的连接弧重复，之前的弧的长度都是小于rules.maxTransMint的
				if (next->depStation == cur->arvStation
					&& horizonRules.maxTransMin < len && len <= horizonRules.maxOutRestMin
					&& next->nodeType != NodeType::baseD)
				{
					Arc* rest_arc = new Arc();
					rest_arc->startNode = cur;
					rest_arc->endNode = next;					
					rest_arc->len = len;
					rest_arc->arcType = ArcType::rest;
					arcSet->emplace_back(rest_arc);
					cur->outArcSet->emplace_back(rest_arc);
					next->inArcSet->emplace_back(rest_arc);
				}
				//必须对copyNodeSet排序后，才能执行这步，因为一旦len > maxOutRestMint后，之后的都大于
				if (len > horizonRules.maxOutRestMin) {
					break;
				}
			}
		}

	}

	std::sort(copyNodeSet.begin(), copyNodeSet.end(),
		[](const Node *a, const Node *b) { return a->endDtUtc < b->endDtUtc; });	
	for (int i = begin; i < end; i++) {
		cur = (*nodeSet)[i];
		if (cur->inArcSet->size() <= 2) {
			for ( int j = i; j > begin; j--) {
				pre = (*nodeSet)[j];
				len = (cur->startDtUtc - pre->endDtUtc) / 60;
				//rules.maxTransMint < intervalMin是为了保证不和之前的连接弧重复，之前的弧的长度都是小于rules.maxTransMint的
				if (pre->arvStation == cur->depStation
					&& horizonRules.maxTransMin < len && len <= horizonRules.maxOutRestMin
					&& pre->nodeType != NodeType::baseO)
				{
					Arc* rest_arc = new Arc();
					rest_arc->startNode = pre;
					rest_arc->endNode = cur;
					rest_arc->len = len;
					rest_arc->arcType = ArcType::rest;
					arcSet->emplace_back(rest_arc);
					pre->outArcSet->emplace_back(rest_arc);
					cur->inArcSet->emplace_back(rest_arc);
				}
				if (len > horizonRules.maxOutRestMin) {
					break;
				}
			}
		}
	}
}

void Network::removeIsolatedNodes()
{
	std::ofstream isolatedNodeFile;
	isolatedNodeFile.open("isolatedFlights_output.csv", std::ios::out);
	
	flightParser flt_csv_parser;
	std::vector<string> headers = flt_csv_parser.getDefaultHeaders();
	for (const auto& col : headers)
	{
		isolatedNodeFile << col << ",";
	}
	isolatedNodeFile << "\n";
	int begin = 2;
	Node* cur;
	bool finished;
	do {
		finished = true;
		for (int i = begin; i < nodeSet->size(); i++) {
			cur = (*nodeSet)[i];
			if (cur->outArcSet->size() == 0) {
				isolatedNodeFile << flt_csv_parser.toCsv(headers, cur->segment);
				// 对出弧为0的点和与其连接的点，删掉之间的弧
				for (auto arc = cur->inArcSet->begin(); arc != cur->inArcSet->end();) {
					auto out_arc_vec = (*arc)->startNode->outArcSet;
					for (auto outarc = out_arc_vec->begin(); outarc != out_arc_vec->end();) {
						if (*outarc == *arc) {
							out_arc_vec->erase(outarc);
							break;
						}
						++outarc;
					}
					auto pos = std::find(arcSet->begin(), arcSet->end(), *arc);
					arcSet->erase(pos);//从弧集中也得删除
					arc = cur->inArcSet->erase(arc);
				}
				nodeSet->erase(nodeSet->begin() + i--);				
				finished = false;
			}
			else if (cur->inArcSet->size() == 0) {
				isolatedNodeFile << flt_csv_parser.toCsv(headers, cur->segment);

				for (auto arc = cur->outArcSet->begin(); arc != cur->outArcSet->end();) {
					auto in_arc_vec = (*arc)->endNode->inArcSet;
					for (auto inarc = in_arc_vec->begin(); inarc != in_arc_vec->end();) {
						if (*inarc == *arc) {
							in_arc_vec->erase(inarc);
							break;
						}
						++inarc;
					}
					auto pos = std::find(arcSet->begin(), arcSet->end(), *arc);
					arcSet->erase(pos);
					arc = cur->outArcSet->erase(arc);
				}
				nodeSet->erase(nodeSet->begin() + i--);				
				finished = false;
			}
		}
	
	} while (!finished);
	isolatedNodeFile.close();
}
void Network::renewResourceTime() {
	Node* resource = nodeSet->front();

	time_t firstDay = (*(nodeSet->begin() + 2 + 2 * baseSet->size()))->startDtLoc;		
	firstDay = getStartTimeOfDay(firstDay);
	resource->startDtUtc = firstDay;
	resource->endDtUtc = firstDay;
}

void Network::createFirstDayArcs() {

	Node* resource = nodeSet->front();
	resource->outArcSet->clear();	

	Node *seg_node;
	int  seg_begin = 2 + 2 * baseSet->size();
	time_t firstDay = nodeSet->back()->endDtUtc;
	for (int i = seg_begin; i < nodeSet->size(); i++) {
		firstDay = (*nodeSet)[i]->startDtLoc < firstDay ? (*nodeSet)[i]->startDtLoc : firstDay;
	}
	firstDay = getStartTimeOfDay(firstDay);
	
	resource->startDtUtc = firstDay;
	resource->endDtUtc = firstDay;
	
	time_t segInDay = 0;

	for (int i = seg_begin; i < nodeSet->size(); i++) {
		seg_node = (*nodeSet)[i];
		segInDay = getStartTimeOfDay(seg_node->startDtLoc);

		if (segInDay == firstDay) {
			Arc* out_arc = new Arc();
			out_arc->startNode = resource;
			out_arc->endNode = seg_node;
			out_arc->len = 0;//(seg_node->startDtLoc - segInDay) / 60;
			out_arc->arcType = ArcType::resourceOut;
			arcSet->emplace_back(out_arc);
			resource->outArcSet->emplace_back(out_arc);
			seg_node->inArcSet->emplace_back(out_arc);
		}
	}
}