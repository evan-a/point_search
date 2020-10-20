// point_search.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"

#define DLLExport extern "C" __declspec(dllexport)

struct SearchContext {
	std::map<int32_t, Point> points;
};

DLLExport SearchContext* create(const Point* points_begin, const Point* points_end)
{
	SearchContext* sc = new SearchContext();
	for (auto point = points_begin; point != points_end; point++)
	{
		sc->points[point->rank] = *point;
	}
	return sc;
}

DLLExport int32_t search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	if (sc == nullptr)
		return 0;

	int32_t out_count = 0;
	for (auto it : sc->points)
	{
		const Point& point = it.second;
		if (point.x >= rect.lx && point.x <= rect.hx && point.y >= rect.ly && point.y <= rect.hy)
		{
			out_points[out_count++] = point;
			if (out_count == count)
				break;
		}
	}
	return out_count;
}

DLLExport SearchContext* destroy(SearchContext* sc)
{
	delete sc;
	return nullptr;
}

