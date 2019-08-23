#pragma once
#ifndef COLUMN_H
#define COLUMN_H
#include "Seg_Path.h"
#include "Crew_Path.h"

enum ColumnType 
{
	relax = 0,
	duty = 1
};

class Column 
{
public:
	Column() {
		cost = FIXED_COST;
		type = ColumnType::duty;
	};
	Column(SegPath& segPath, CrewGroup& crweGroup) {
		_segpath = &segPath;
		_crewgroup = &crweGroup;

		cost = FIXED_COST;
		type = ColumnType::duty;
	}
	~Column() {
		_segpath = NULL;
		_crewgroup = NULL;
	}
	double cost;
	const int FIXED_COST = 0;//100; //¹Ì¶¨³É±¾
	double reduced_cost;

	SegPath* _segpath;
	CrewGroup* _crewgroup;

	ColumnType type;
};
using ColumnPool = std::vector<Column*>;

//class ColumnPool
//{
//public:
//	std::vector<Column*> column_pool;
//};

#endif // !COLUMN_H