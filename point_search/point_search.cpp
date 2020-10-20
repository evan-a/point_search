// point_search.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "point_search.h"
#include "sqlite3.h"
#include <map>

#define DLLExport extern "C" __declspec(dllexport)

struct SearchContext 
{
	sqlite3* ppDb        = nullptr;
	sqlite3_stmt* pQuery = nullptr;
	std::map<int32_t, Point> point_map;
};

DLLExport SearchContext* create(const Point* points_begin, const Point* points_end)
{
	int rc = sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
	rc = sqlite3_initialize();

	auto sc = new SearchContext();
	//rc = sqlite3_open(":memory:", &sc->ppDb);
	rc = sqlite3_open(":memory:", &sc->ppDb);
	if (rc != SQLITE_OK)
	{
		delete sc;
		return nullptr;
	}

	rc = sqlite3_exec(sc->ppDb, "DROP TABLE IF EXISTS points", nullptr, nullptr, nullptr);

	rc = sqlite3_exec(sc->ppDb, "CREATE VIRTUAL TABLE points USING rtree (rank, minX, maxX, minY, maxY, +id integer);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		delete sc;
		return nullptr;
	}

	rc = sqlite3_exec(sc->ppDb, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

	sqlite3_stmt* insert_stmt;
	rc = sqlite3_prepare_v2(sc->ppDb, "INSERT INTO points VALUES(?1, ?2, ?2, ?3, ?3, ?4);", -1, &insert_stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		delete sc;
		return nullptr;
	}

	//uint64_t point_count = (reinterpret_cast<const uint64_t>(points_end) - reinterpret_cast<const uint64_t>(points_begin)) / sizeof(Point);
	int count = 0;
	for (auto point = points_begin; point != points_end; point++)
	{
		rc = sqlite3_reset(insert_stmt);
		rc = sqlite3_bind_int(    insert_stmt, 1, point->rank);
		rc = sqlite3_bind_double( insert_stmt, 2, point->x);
		rc = sqlite3_bind_double( insert_stmt, 3, point->y);
		rc = sqlite3_bind_double( insert_stmt, 4, point->id);

		rc = sqlite3_step(insert_stmt);

		count++;
	}
	rc = sqlite3_finalize(insert_stmt);

	rc = sqlite3_exec(sc->ppDb, "END TRANSACTION", nullptr, nullptr, nullptr);

	rc = sqlite3_prepare_v2(sc->ppDb, "SELECT rank, minX, minY, id FROM points WHERE minX>=?1 AND maxX<=?2 AND minY>=?3 AND maxY<=?4 ORDER BY rank ASC;", -1, &sc->pQuery, nullptr);
	if (rc != SQLITE_OK)
	{
		delete sc;
		return nullptr;
	}

	return sc;
}

DLLExport int32_t search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	if (sc == nullptr || sc->ppDb == nullptr || sc->pQuery == nullptr)
	{
		return 0;
	}

	int rc = sqlite3_reset(sc->pQuery);
	rc = sqlite3_bind_double(sc->pQuery, 1, rect.lx);
	rc = sqlite3_bind_double(sc->pQuery, 2, rect.hx);
	rc = sqlite3_bind_double(sc->pQuery, 3, rect.ly);
	rc = sqlite3_bind_double(sc->pQuery, 4, rect.hy);

	sc->point_map.clear();
	int result_count = 0;
	rc = sqlite3_step(sc->pQuery);
	while (rc == SQLITE_ROW && result_count < count)
	{

		// Search
		out_points[result_count].rank = sqlite3_column_int(sc->pQuery, 0);
		out_points[result_count].x = static_cast<float>(sqlite3_column_double(sc->pQuery, 1));
		out_points[result_count].y = static_cast<float>(sqlite3_column_double(sc->pQuery, 2));
		out_points[result_count].id = static_cast<int8_t>(sqlite3_column_int(sc->pQuery, 3));
		result_count++;

		rc = sqlite3_step(sc->pQuery);
	}

	return result_count;
}

DLLExport SearchContext* destroy(SearchContext* src)
{
	if (src)
	{
		if (src->ppDb)
		{
			if (src->pQuery)
			{
				sqlite3_finalize(src->pQuery);
				src->pQuery = nullptr;
			}
			sqlite3_close(src->ppDb);
		}
	}
	delete src;
	src = nullptr;

	sqlite3_shutdown();

	return src;
}



