// point_search.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "point_search.h"

#define DLLExport extern "C" __declspec(dllexport)

struct SearchContext {

};

DLLExport SearchContext* create(const Point* points_begin, const Point* points_end)
{
	return new SearchContext();
}

DLLExport int32_t search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	return 0;
}

DLLExport SearchContext* destroy(SearchContext* sc)
{
	delete sc;
	return nullptr;
}

