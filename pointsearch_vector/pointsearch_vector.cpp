// point_search.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"

#define DLLExport extern "C" __declspec(dllexport)

struct SearchContext {
	std::vector<Point> points;
};

DLLExport SearchContext* create(const Point* points_begin, const Point* points_end)
{
	SearchContext* sc = new SearchContext();

	// Put the points in the vector
	sc->points.assign(points_begin, points_end);

	// Sort the points by rank, from lowest number to highest
	std::sort(sc->points.begin(), sc->points.end(), [](const Point& a, const Point& b) {
		return a.rank < b.rank;
	});

	return sc;
}

DLLExport int32_t search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	if (sc == nullptr)
		return 0;

	int32_t idx = 0;
	for (size_t i = 0; i < sc->points.size(); i++)
	{
		const Point& pt = sc->points[i];
		// Binary & seems to be faster than logical &&
		if ((pt.x >= rect.lx) & (pt.x <= rect.hx) & (pt.y >= rect.ly) & (pt.y <= rect.hy))
		{
			out_points[idx++] = pt;
			if (idx == count)
				break;
		}
	}
	return idx;
}

DLLExport SearchContext* destroy(SearchContext* sc)
{
	delete sc;
	return nullptr;
}

