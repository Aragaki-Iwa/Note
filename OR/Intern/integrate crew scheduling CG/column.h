#pragma once
#ifndef COLUMN_H
#define COLUMN_H
#include "Seg_Path.h"
#include "Crew_Path.h"


class Column 
{
public:
	Column() {};
	Column(SegPath& segPath, CrewGroup& crweGroup) {
		_segpath = &segPath;
		_crewgroup = &crweGroup;
	}
	~Column() {
		_segpath = NULL;
		_crewgroup = NULL;
	}
	double cost;
	double reduced_cost;

	SegPath* _segpath;
	CrewGroup* _crewgroup;

	std::string type = "";
};
using ColumnPool = std::vector<Column*>;

//class ColumnPool
//{
//public:
//	std::vector<Column*> column_pool;
//};

#endif // !COLUMN_H