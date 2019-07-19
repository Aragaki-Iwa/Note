#include "FindDutySet.h"

std::vector<Path*>* PathFinder::getPathSet() {
	return &_pathSet;
}

void PathFinder::findPathSet(Network* net, vector<Node*>& startNodeSet, const CrewRules& rules)
{		
	//debug
	/*std::vector<Node*> copyNodeSet(*net->nodeSet);
	std::sort(copyNodeSet.begin(), copyNodeSet.end(),
		[](const Node *a, const Node *b) { return a->startDtUtc < b->startDtUtc; });
	*/


	//std::vector<string> bases;
	//for (int i = 2; i < 2 + 2 * net->baseSet->size(); i++) {
	///*	if (std::find(bases.begin(), bases.end(), (*net->nodeSet)[i]->depStation) == bases.end()) {
	//		bases.emplace_back((*net->nodeSet)[i]->depStation);
	//	}*/
	//	startNodeSet.emplace_back((*net->nodeSet)[i++]);
	//}
	//string firstday;
	//for (int i = 0; i < copyNodeSet.size(); i++) {
	//	if (copyNodeSet[i]->nodeType == NodeType::seg || copyNodeSet[i]->nodeType == NodeType::dhd) {
	//		firstday = copyNodeSet[i]->segment->getDate();
	//		break;
	//	}
	//}	
	//for (int i = 1 + net->baseSet->size(); i < copyNodeSet.size(); i++) {
	//	//startNodeSet.emplace_back((*net->nodeSet)[i++]);//ֻȡbase_o
	//	//ȡ��һ��ӻ��س�����segment
	//	if ((copyNodeSet[i]->nodeType == NodeType::seg || copyNodeSet[i]->nodeType == NodeType::dhd)
	//		&& copyNodeSet[i]->segment->getDate() == firstday
	//		&& std::find(bases.begin(),bases.end(),copyNodeSet[i]->depStation) != bases.end()) {
	//		startNodeSet.emplace_back(copyNodeSet[i]);
	//	}	
	//}
	//��Щ��ֵӦ����net�Լ�����
	//ֻ���Ϊ�������ߵģ�if(node->assigned == false)
	_pathSet.clear();
	
	Node* node;
	for (int i = 2 ; i < net->nodeSet->size(); i++) {
		node = (*net->nodeSet)[i];
		if (node->visited == false) {
			Label* label = new Label();
			label->preArc = (*node->inArcSet)[0];			
			node->labelSet.emplace_back(label);
		}		
	}
	for (auto& arc : *net->arcSet) {
		_VisitedByPath[arc] = 0;
	}

	/*Label* label;
	for (int i = 2 + 2 * net->baseSet->size(); i < net->nodeSet->size(); i++) {
		node = (*net->nodeSet)[i];
		if (node->visited == false) {
			label = node->labelSet.front();
			label->preLabel = label->preArc->startNode->labelSet.front();
		}
	}*/
	
	//end !debug
		
	//for (int i = 0; i < startNodeSet.size(); i++)
	//{
	//	initStartNode(startNodeSet[i]);
	//	//dfs(startNodeSet[i], rules);
	//	for (auto& node : *net->nodeSet) {
	//		node->visited = false;
	//	}
	//	//dfs_nonrecursive(startNodeSet[i], rules);
	//	dfs_nonrecursive2(startNodeSet[i], rules);
	//}

	//startNodeSet���ⲿ����
	//ʵ��createCurResource()�Ĺ���ҲӦ�����ⲿ�����ⲿ�����ֻ��һ��"cur_resource"���
	//startNodeSet.emplace_back(net->nodeSet->front());
	Node* resource = createCurResource(startNodeSet);
	initResourceNode(resource);
	for (auto& node : *net->nodeSet) {
		node->visited = false; 
	}
	dfs_nonrecursive2(resource, rules);



	for (auto& label : resource->labelSet) {
		delete label;
	}
	for (auto& out_arc : *resource->outArcSet) {
		out_arc->endNode->inArcSet->pop_back();
		_VisitedByPath.erase(out_arc);
		delete out_arc;
	}
	delete resource;

	for (int i = 2; i < net->nodeSet->size(); i++) {
		node = (*net->nodeSet)[i];					
		for (auto& label : node->labelSet) {
			delete label;
		}
		node->labelSet.clear();		
	}
}

Node* PathFinder::createCurResource(std::vector<Node*>& startNodeSet) {
	Node* resource = new Node();
	resource->arvStation = "cur_resource";
	resource->depStation = "cur_resource";
	resource->startDtUtc = 0;
	resource->startDtLoc = 0;
	resource->endDtUtc = 0;
	resource->endDtLoc = 0;
	resource->taskMin = 0;
	resource->nodeType = NodeType::resource;
	resource->name = "cur_resource";
	resource->visited = false;
	resource->assigned = false;
	
	for (auto& node : startNodeSet) {
		//Ҫ��node���ߵ���㶼ָ���½���cur_resource
		//resource->outArcSet->insert(resource->outArcSet->end(), node->outArcSet->begin(), node->outArcSet->end());
		Arc* out_arc = new Arc();
		out_arc->startNode = resource;
		out_arc->endNode = node;
		out_arc->len = 0;
		resource->outArcSet->emplace_back(out_arc);
		node->inArcSet->emplace_back(out_arc);//�����������·��pop

		_VisitedByPath[out_arc] = 0;
		/*for (auto& out_arc : *node->outArcSet) {
			out_arc->startNode = resource;
			out_arc->len = 0;
			resource->outArcSet->emplace_back(out_arc);
		}*/
	}
	return resource;
}

void PathFinder::initResourceNode(Node* startNode) {
	startNode->becameStartNode = true;
	startNode->visited = false;	
	startNode->labelSet.clear();
	Label* label = new Label();
	//label->preArc = startNode->inArcSet->front();
	label->preLabel = new Label();	
	startNode->labelSet.emplace_back(label);
	for (auto& arc : *startNode->outArcSet) {
		arc->endNode->labelSet.front()->preLabel = label;
	}

}
void PathFinder::dfs(Node* startNode, const CrewRules& rules)
{
	startNode->visited = true;
	static  bool prune = false;
	std::cout << "_arcList.size = " << _arcList.size() << "\n";
	std::cout << "....................._pathSet.size = " << _pathSet.size() << "\n";

	if (checkTermination(*startNode->labelSet.front()->preLabel, *startNode->labelSet.front(), rules)) {
		Node* endNode = startNode->labelSet.front()->preArc->startNode;//_arcList.back()->endNode;
		//startNode�ǵ�ǰ���һ�������յ㣬�����ǲ����еģ�����duty�Ľ�����endNode��ջ�����һ���������
		Label* endLabel = endNode->labelSet.front(); //������ȣ�����ֻ����һ��label���ڵ���dfs����֮ǰ���ȶ�ÿ��node new label
		_pathSet.emplace_back(backtrack(endLabel));
		std::cout << "-----------------------\n";
		std::cout << "find "<< _pathSet.size() <<"th path, endNode = " << endNode->segment->getFlightNumber() << "\n";
		std::cout << _pathSet.back()->flyMin << ", " << _pathSet.back()->workMin << "\n";
		
		std::cout << "-----------------------\n";
		endNode->visited = false;
		_arcList.pop_back();		
		prune = true;

	}
	else {	
		Arc* arc = 0;
		for (int i = 0; i < startNode->outArcSet->size(); i++) {
			arc = startNode->outArcSet->at(i);
			_arcList.emplace_back(arc);
			if (arc->endNode->visited == false) {
				arc->endNode->labelSet.front()->preLabel = startNode->labelSet.front();
				arc->endNode->labelSet.front()->preArc = arc;
				extend(startNode->labelSet.front(), arc, arc->endNode->labelSet.front());
				
				if (checkFeasible(*startNode->labelSet.front(), arc, arc->endNode->labelSet.front(), rules)) {
					if (startNode->nodeType != NodeType::baseO) {
						//std::cout << "��ǰ�ڵ�" << startNode->segment->getFlightNumber();							
					}
					else {
						std::cout << startNode->name;
					}
					if (arc->endNode->nodeType != NodeType::baseD) {
						//std::cout << " �ĺ�̽ڵ�: " << arc->endNode->segment->getDate() << "^" << arc->endNode->segment->getFlightNumber() << "\n";
					}
					else {
						//std::cout << arc->endNode->name << "\n";
					}

					//std::cout << "feasibel and not meetend\n";
					updateLabel(startNode->labelSet.front(), arc, arc->endNode->labelSet.front());
					dfs(arc->endNode, rules);
					if (prune) {						
						prune = false;
						break;
					}
					//std::cout << "end of feasibel and not meetend\n";
				}
				else {
					_arcList.back()->endNode->visited = false;
					_arcList.pop_back();
					if (arc->startNode->nodeType == NodeType::baseO || arc->endNode->nodeType == NodeType::baseD) {
						std::cout << arc->startNode->name;
					}
					else {
						//std::cout << arc->startNode->segment->getFlightNumber() << "��" << arc->endNode->segment->getFlightNumber() << "������\n";
					}
				}
			}
		}
		if (!_arcList.empty()) {
			_arcList.back()->endNode->visited = false;
			_arcList.pop_back();
		}
	}
}
void PathFinder::dfs_nonrecursive(Node* startNode, const CrewRules& rules) {
	
	std::stack<Node*> nodeStack;
	startNode->visited = true;	
	nodeStack.push(startNode);
	Node* s;
	Node* nextNode;
	Arc* arc;
		
	while (!nodeStack.empty()) {
		s = nodeStack.top();
		
		if (_pathSet.size() == 30) {
			int y = 0;
		}

		std::cout << "cur base" << startNode->depStation << "\t";
		/*if (s->nodeType == NodeType::seg || s->nodeType == NodeType::dhd) {
			std::cout << "cur s node:" << s->segment->getFlightNumber() << "\n";
		}
		else {
			std::cout << "cur s node:" << s->name << "\n";
		}
		std::cout << "cur node address " << s << "\n";*/
		std::cout << "_pathSet.size = " << _pathSet.size() << "\n";

		if (checkTermination(*s->labelSet.front()->preLabel, *s->labelSet.front(), rules)) {
			Node* endNode = s->labelSet.front()->preArc->startNode;
			//startNode�ǵ�ǰ���һ�������յ㣬�����ǲ����еģ�����duty�Ľ�����endNode��ջ�����һ���������
			Label* endLabel = endNode->labelSet.front(); //������ȣ�����ֻ����һ��label���ڵ���dfs����֮ǰ���ȶ�ÿ��node new label
			Path* new_path = backtrack(endLabel);
			//�ж��Ƿ��ظ�����·
			/*int flag = new_path->route.size();
			for (int i = _pathSet.size() - 2; i >= 0; i--) {
				if (new_path->route.size() == _pathSet[i]->route.size()) {
					for (int j = new_path->route.size() - 1; j >=0 ; j--) {
						if (new_path->route[j] == _pathSet[i]->route[j]) {
							flag--;
						}
					}
				}
				
			}
			if (flag == 0) {
				std::cout << "---------------------ERROR:exist path-----------------------\n";
			}*/


			if (_pathSet.size() > 0 && new_path->route.back() != _pathSet.back()->route.back()) {
				_pathSet.emplace_back(new_path);			
			}
			else if (_pathSet.size() == 0) {
				_pathSet.emplace_back(new_path);
			}
			
			

			/*std::cout << "-----------------------\n";
			std::cout << "find " << _pathSet.size() << "th path, endNode = " << endNode->segment->getFlightNumber() << "\n";
			std::cout << "flyMin:" << _pathSet.back()->flyMin << ", workMin:" << _pathSet.back()->workMin << "\n";
			std::cout << "-----------------------\n";*/

			//endNode->visited = false;
			/*for (int i = 0; i < endNode->outArcSet->size(); i++) {
				arc = (*endNode->outArcSet)[i];
				nextNode = arc->endNode;
				_VisitedByPath[arc] = 0;
			}*/

			s->visited = false;
			_VisitedByPath[s->labelSet.front()->preArc] = 1;
			nodeStack.pop();
			//nodeStack.pop();
		}
		else {		
			int i = 0;
			for (i = 0; i < s->outArcSet->size(); i++) {
				arc = (*s->outArcSet)[i];
				nextNode = arc->endNode;
				int v = _VisitedByPath[arc];
				if (nextNode->visited == false && v == 0 && nextNode->nodeType != NodeType::baseD) {
					nextNode->labelSet.front()->preArc = arc;
					nextNode->labelSet.front()->preLabel = s->labelSet.front();

					extend(s->labelSet.front(), arc, nextNode->labelSet.front());
					Node* dhdNode = findDeadhead(nextNode->labelSet.front());
					if (dhdNode == NULL) {
						updateLabel(s->labelSet.front(), arc, arc->endNode->labelSet.front());
						nodeStack.push(nextNode);
						nextNode->visited = true;
						break;
					}
					else {
						_VisitedByPath[arc] = 1;
						nextNode->visited = false;
						continue;
					}
				}				
			}
			if (i == s->outArcSet->size()) { //˵���ڵ�s�����г��߱�������,�ָ������ڽӵ�Ϊδ��·������
				for (i = 0; i < s->outArcSet->size(); i++) {
					arc = (*s->outArcSet)[i];
					nextNode = arc->endNode;
					_VisitedByPath[arc] = 0;
				}
				s->visited = false;
				_VisitedByPath[s->labelSet.front()->preArc] = 1;
				nodeStack.pop();
			}

		}
	}
}
	

void PathFinder::dfs_nonrecursive2(Node* startNode, const CrewRules& rules) {
	std::stack<Node*> nodeStack;
	startNode->visited = true;
	nodeStack.push(startNode);
	Node* s;
	Node* nextNode = nullptr;
	Arc* arc;

	while (!nodeStack.empty()) {
		s = nodeStack.top();

		int i = 0;
		for (i = 0; i < s->outArcSet->size(); i++) {
			arc = (*s->outArcSet)[i];
			nextNode = arc->endNode;
			int v = _VisitedByPath[arc];
			if (nextNode->visited == false && v == 0 && nextNode->nodeType != NodeType::baseD) {
				nextNode->labelSet.front()->preArc = arc;
				nextNode->labelSet.front()->preLabel = s->labelSet.front();

				extend(s->labelSet.front(), arc, nextNode->labelSet.front());
				Node* dhdNode = findDeadhead(nextNode->labelSet.front());
				if (dhdNode == NULL) {
					updateLabel(s->labelSet.front(), arc, arc->endNode->labelSet.front());
					nodeStack.push(nextNode);
					nextNode->visited = true;					
					break;
				}
				else {
					_VisitedByPath[arc] = 1;
					nextNode->visited = false;
					continue;
				}
			}
		}
		if (i == s->outArcSet->size()) { //˵���ڵ�s�����г��߱�������,�ָ������ڽӵ�Ϊδ��·������
			for (i = 0; i < s->outArcSet->size(); i++) {
				arc = (*s->outArcSet)[i];
				nextNode = arc->endNode;
				_VisitedByPath[arc] = 0;
			}
			s->visited = false;
			_VisitedByPath[s->labelSet.front()->preArc] = 1;
			nodeStack.pop();
			continue;			
		}
		//std::cout << "cur base" << startNode->depStation << "\t";
		/*if (s->nodeType == NodeType::seg || s->nodeType == NodeType::dhd) {
			std::cout << "cur s node:" << s->segment->getFlightNumber() << "\n";
		}
		else {
			std::cout << "cur s node:" << s->name << "\n";
		}
		std::cout << "cur node address " << s << "\n";*/
		//std::cout << "_pathSet.size = " << _pathSet.size() << "\n";

		if (checkTermination(*nextNode->labelSet.front()->preLabel, *nextNode->labelSet.front(), rules)) {
			//��sΪ�����㣬nextNodeΪ����㣬�ж��Ƿ�����죬��nextNode�Ƿ���һ�����е��յ�
			Label* endLabel = nextNode->labelSet.front();
			Path* new_path = backtrack(endLabel);

			_pathSet.emplace_back(new_path);
			/*std::cout << "-----------------------\n";
			std::cout << "find " << _pathSet.size() << "th path, endNode = " << endNode->segment->getFlightNumber() << "\n";
			std::cout << "flyMin:" << _pathSet.back()->flyMin << ", workMin:" << _pathSet.back()->workMin << "\n";
			std::cout << "-----------------------\n";*/
			
			_VisitedByPath[nextNode->labelSet.front()->preArc] = 1;
			
		}
		else if ((nextNode->labelSet.front()->accumuFlyMin > rules.horizon_rules->maxFlyMin
			|| nextNode->labelSet.front()->accumuWorkMin > rules.horizon_rules->maxDutyMin)) {
			nextNode->visited = false;
			_VisitedByPath[nextNode->labelSet.front()->preArc] = 1;
			nodeStack.pop();
		}		
	}
}


void PathFinder::extend(const Label* label, Arc* curArc, Label* nextLabel)
{
	nextLabel->accumuFlyMin = label->accumuFlyMin + curArc->endNode->taskMin; //flyMin = sum of fly time
	if (curArc->arcType != ArcType::baseOut) {
		nextLabel->accumuWorkMin = label->accumuWorkMin + curArc->len + curArc->endNode->taskMin;//equals flyMin + connect time(arc.len)			
	}
	else {
		nextLabel->accumuWorkMin = nextLabel->accumuFlyMin;
	}
	
}
bool PathFinder::checkFeasible(const Label& label, Arc* curArc, Label* nextLabel, const CrewRules& crewRules)
{				
	//û���ð���������������㳤�Ⱥ�ͽ�ֹ�����Կ�����ֻ��Ҫ���deadhead	
	/*if (nextLabel->accumuFlyMin > crewRules.horizon_rules->maxFlyMin) {
		return false;
	}
	else if (nextLabel->accumuWorkMin > crewRules.horizon_rules->maxDutyMin) {
		return false;
	}*/
	if (checkDeadhead(nextLabel) == false ) {
		return false;
	}
	if (nextLabel->preArc->endNode->nodeType == NodeType::baseD) { //δterminate��Ȼ���Ѿ�������baseD
		return false;
	}
	
	return true;
}
bool PathFinder::checkDeadhead(Label* nextLabel)
{
	Label* label = nextLabel;
	Node* node = label->preArc->endNode;
	//Node* node = label->preArc->startNode;	
	if (label->preArc->startNode->nodeType == NodeType::baseO) {
		return true;
	}
	while (node->becameStartNode == false) {
		if (node->nodeType == NodeType::dhd) {
			return false;
		}
		label = label->preLabel;
		//node = label->preArc->startNode; 
		node = label->preArc->endNode;
	}
	return true;
}
Node* PathFinder::findDeadhead(Label* nextLabel) {
	Label* label = nextLabel;
	Node* node = label->preArc->endNode;

	/*if (label->preArc->startNode->nodeType == NodeType::baseO) {
		return NULL;
	}*/
	//TODO::TEST
	while (label->preArc->startNode->becameStartNode == false) {
		if (node->nodeType == NodeType::dhd) {
			return node;
		}
		label = label->preLabel;
		node = label->preArc->endNode;
	}
	return NULL;
}

bool PathFinder::checkTermination(const Label& preLabel, const Label& curLabel, const CrewRules& crewRules)
{
	bool finished = false;
	/*if ((preLabel.accumuFlyMin >= crewRules.horizon_rules->minFlyMin && preLabel.accumuFlyMin <= crewRules.horizon_rules->maxFlyMin
			&& preLabel.accumuWorkMin >= crewRules.horizon_rules->minDutyMin && preLabel.accumuWorkMin <= crewRules.horizon_rules->maxDutyMin)
		&& (curLabel.accumuWorkMin >= crewRules.horizon_rules->maxDutyMin || curLabel.accumuFlyMin >= crewRules.horizon_rules->maxFlyMin)) {*/
	if ((curLabel.accumuFlyMin >= crewRules.horizon_rules->minFlyMin && curLabel.accumuFlyMin <= crewRules.horizon_rules->maxFlyMin
		&& curLabel.accumuWorkMin >= crewRules.horizon_rules->minDutyMin && curLabel.accumuWorkMin <= crewRules.horizon_rules->maxDutyMin)){
		//recur2
		finished = true;
	}	

	return finished;
}


void PathFinder::updateLabel(Label* label, Arc* arc, Label* nextLabel)//�����Լ�飬���к󣬸����±�ŵ����ԣ�arc��
{
	nextLabel->preLabel = label;
	nextLabel->preArc = arc;
}
Path* PathFinder::backtrack(Label* endLabel)
{
	Path* path = new Path();
	Label* label = endLabel;
	Arc* arc = endLabel->preArc;
	while (!arc->startNode->becameStartNode)
	{
		path->route.emplace_back(arc->endNode);
		label = label->preLabel;
		arc = label->preArc;
	}
	path->route.emplace_back(arc->endNode);
	std::reverse(path->route.begin(), path->route.end());

	path->flyMin = endLabel->accumuFlyMin;
	path->workMin = endLabel->accumuWorkMin;

	Node* start_node, *end_node;
	start_node = path->route.front();
	end_node = path->route.back();
	path->startDtLoc = start_node->startDtLoc;
	path->endDtLoc = end_node->endDtLoc;
	path->startStation = start_node->depStation;
	path->endStation = end_node->arvStation;
	
	path->startDate = start_node->segment->getDate();

	return path;
}