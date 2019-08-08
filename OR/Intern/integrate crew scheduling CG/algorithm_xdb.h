#pragma once
#ifndef ALGORITHM_XDB_H
#define ALGORITHM_XDB_H
#include <algorithm>
#include <iterator>
#include <vector>

//! return true if <vec> contains <elem>
template<class T>
bool isContain(const std::vector<T>& vec, T elem) {
	if (std::find(vec.begin(), vec.emd(), elem) == vec.end()) {
		return false;
	}
	return true;
}

//! return false if <vec> not contains <elem> 
template<class T>
bool removeElem(std::vector<T>* vecPtr, T elem) {
	std::vector<T>::iterator it = vecPtr->begin();
	for (; it != vecPtr->end();) {
		if (*it == elem) {
			it = vecPtr->erase(it);
		}
		else {
			++it;
		}
	}

	if (it == vecPtr->end()) {
		return false;
	}
	return true;
}
#endif // !ALGORITHM_XDB_H