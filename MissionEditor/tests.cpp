/*
	FinalSun/FinalAlert 2 Mission Editor

	Copyright (C) 1999-2024 Electronic Arts, Inc.
	Authored by Matthias Wagner

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version BR of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "tests.h"
#include "inlines.h"
#include <string>
#include <functional>
#include <iostream>
#include "Tube.h"

class TestError : public std::runtime_error
{
public:
	TestError(const std::string text) : std::runtime_error(text)
	{

	}
};

bool RaiseTestError(const char* file, int line, const char* function, const char* assertion)
{
	const std::string error = std::string("Test assertion: ") + assertion + " in file " + file + ", line " + std::to_string(line);
	throw TestError(error);
	return false;
}

bool ReportTest(const char* file, int line, const char* function, const char* assertion)
{
	const std::string error = std::string("Test succeeded in ") + function + " " + assertion + " in file " + file + ", line " + std::to_string(line);
	std::cout << error << std::endl;
	return true;
}

#define STR( s ) # s
#define REPORT_TEST(COND) (void) (((!!(COND)) && ReportTest(THIS_FILE, __LINE__, __FUNCTION__, STR(COND) )) || RaiseTestError(THIS_FILE, __LINE__, __FUNCTION__, STR(COND)))
#define TEST(COND) (void) ((!!(COND)) || RaiseTestError(THIS_FILE, __LINE__, __FUNCTION__, STR(COND)))

bool run_test(const std::function<void()>& f)
{
	try
	{
		f();
		return true;
	}
	catch(const TestError& e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "ERROR: Exception occurred: " << e.what() << std::endl;
	}
	return false;
}

int main(int argc, char* argv[])
{
	Tests t;
	return t.run();
}

int Tests::run()
{
	int failed_tests = 0;
	std::vector<std::function<void()>> test_functions({ 
		[this]() { test_inlines(); },
		[this]() { test_tube_create(); },
		[this]() { test_tube_reverse(); },
		[this]() { test_tube_append(); },
		[this]() { test_tube_delimiter(); },
		[this]() { test_hsv(); },
		[this]() { test_iso(); },
	});
	for (const auto f : test_functions)
	{
		if (!run_test(f))
			++failed_tests;
	}

	std::cout << "Failed: " << failed_tests << std::endl;
	std::cout << "Succeeded: " << test_functions.size() - failed_tests << std::endl;
	return failed_tests ? 1 : 0;
}

void Tests::test_inlines()
{
	REPORT_TEST(GetParam("SOME,,Value", 1) == CString(""));
	REPORT_TEST(GetParam("SOME,,Value", 2) == CString("Value"));
	REPORT_TEST(GetParam("SOME,,Value", 0) == CString("SOME"));
	REPORT_TEST(GetParam("SOME,,Value", 77) == CString(""));
	REPORT_TEST(GetParam("SOME,,Value,", 3) == CString(""));
	REPORT_TEST(GetParam("SOME,,Value,0", 3) == CString("0"));
	REPORT_TEST(GetParam(" SOME,", 0) == CString(" SOME"));
	REPORT_TEST(SplitParams("SOME,,Value,0") == std::vector<CString>({ "SOME","","Value","0" }));
	REPORT_TEST(SplitParams("") == std::vector<CString>({ "" }));
	REPORT_TEST(SplitParams("SOME,,Value,0,") == std::vector<CString>({ "SOME","","Value","0", "" }));
	REPORT_TEST(Join("::", { "my", "value" }) == "my::value");
	REPORT_TEST(SetParam("SOME,,Value,0,", 0, "NOTSOME") == "NOTSOME,,Value,0,");
	REPORT_TEST(SetParam("SOME,,Value,0,", 1, "NOTSOME") == "SOME,NOTSOME,Value,0,");
	REPORT_TEST(SetParam("SOME,,Value,0,", 3, "1") == "SOME,,Value,1,");
	REPORT_TEST(SetParam("SOME,,Value,0,", 10, "A") == "SOME,,Value,0,,,,,,,A");
}

namespace TubeDirections
{
	auto TL = ETubeDirection::TopLeft;
	auto TC = ETubeDirection::Top;
	auto TR = ETubeDirection::TopRight;
	auto CR = ETubeDirection::Right;
	auto BR = ETubeDirection::BottomRight;
	auto BC = ETubeDirection::Bottom;
	auto BL = ETubeDirection::BottomLeft;
	auto CL = ETubeDirection::Left;
	auto XX = ETubeDirection::Undefined;
}

void Tests::test_tube_create()
{
	using namespace TubeDirections;

	REPORT_TEST(CTube::autocreate(50, 50, 50, 50) == CTube(50, 50, XX, 50, 50, std::vector<ETubeDirection>({ XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 55, 50) == CTube(50, 50, BC, 55, 50, std::vector<ETubeDirection>({ BC, BC, BC, BC, BC, XX})));
	REPORT_TEST(CTube::autocreate(50, 50, 45, 50) == CTube(50, 50, TC, 45, 50, std::vector<ETubeDirection>({ TC, TC, TC, TC, TC, XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 45, 45, 0) == CTube(50, 50, CL, 45, 45, std::vector<ETubeDirection>({ TL, TL, TL, TL, TL, XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 45, 45) == CTube(50, 50, CL, 45, 45, std::vector<ETubeDirection>({ CL, TL, TL, TL, TL, TC, XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 45, 46, 0) == CTube(50, 50, TC, 45, 46, std::vector<ETubeDirection>({ TL, TL, TL, TL, TC, XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 45, 46, 1) == CTube(50, 50, TC, 45, 46, std::vector<ETubeDirection>({ TC, TL, TL, TL, TL, XX })));
	REPORT_TEST(CTube::autocreate(50, 50, 46, 45, 1) == CTube(50, 50, CL, 46, 45, std::vector<ETubeDirection>({ CL, TL, TL, TL, TL, XX })));

}

void Tests::test_tube_reverse()
{
	using namespace TubeDirections;

	REPORT_TEST(CTube(50, 50, BC, 55, 50, std::vector<ETubeDirection>({ BC, BC, BC, BC, BC, XX})).reverse() ==
				CTube(55, 50, TC, 50, 50, std::vector<ETubeDirection>({ TC, TC, TC, TC, TC, XX })));
	REPORT_TEST(CTube(50, 50, TC, 45, 46, std::vector<ETubeDirection>({ TC, TL, TL, TL, TL, XX })).reverse() ==
				CTube(45, 46, BR, 50, 50, std::vector<ETubeDirection>({ BR, BR, BR, BR, BC, XX })));
	REPORT_TEST(CTube(50, 50, BC, 51, 50, std::vector<ETubeDirection>({ BC, XX })).reverse() ==
		        CTube(51, 50, TC, 50, 50, std::vector<ETubeDirection>({ TC, XX })));
	REPORT_TEST(CTube(50, 50, BC, 52, 50, std::vector<ETubeDirection>({ BC, BC, XX })).reverse() ==
				CTube(52, 50, TC, 50, 50, std::vector<ETubeDirection>({ TC, TC, XX })));

}

void Tests::test_tube_append()
{
	using namespace TubeDirections;

	CTube tubeToBottom(50, 50, BC, 53, 50, std::vector<ETubeDirection>({ BC, BC, BC, XX }));
	tubeToBottom.append(55, 50);
	REPORT_TEST(tubeToBottom == CTube(50, 50, BC, 55, 50, std::vector<ETubeDirection>({ BC, BC, BC, BC, BC, XX })));

	CTube tubeToTL(50, 50, TC, 49, 49, std::vector<ETubeDirection>({ TL, XX }));
	tubeToTL.append(49, 47);
	REPORT_TEST(tubeToTL == CTube(50, 50, TC, 49, 47, std::vector<ETubeDirection>({ TL, CL, CL, XX })));

	CTube nullLen(50, 50, XX, 50, 50, std::vector<ETubeDirection>({ XX }));
	nullLen.append(55, 50);
	REPORT_TEST(nullLen == CTube(50, 50, BC, 55, 50, std::vector<ETubeDirection>({ BC, BC, BC, BC, BC, XX })));

	CTube nullLenYMajor(50, 50, XX, 50, 50, std::vector<ETubeDirection>({ XX }));
	nullLenYMajor.append(51, 53);
	REPORT_TEST(nullLenYMajor == CTube(50, 50, CR, 51, 53, std::vector<ETubeDirection>({ CR, BR, CR, XX })));

	CTube nullAppend(50, 50, BC, 52, 50, std::vector<ETubeDirection>({ BC, BC, XX }));
	nullAppend.append(52, 50);
	REPORT_TEST(nullAppend == CTube(50, 50, BC, 52, 50, std::vector<ETubeDirection>({ BC, BC, XX })));

	CTube zeroTube(50, 50, XX, 50, 50, std::vector<ETubeDirection>());
	zeroTube.append(55, 50);
	REPORT_TEST(zeroTube == CTube(50, 50, BC, 55, 50, std::vector<ETubeDirection>({ BC, BC, BC, BC, BC, XX })));

	// Intersection - for now assume TS allows this
	CTube tubeWithIntersection(50, 50, TC, 49, 49, std::vector<ETubeDirection>({ TL, XX }));
	tubeWithIntersection.append(51, 51);
	REPORT_TEST(tubeWithIntersection == CTube(50, 50, TC, 51, 51, std::vector<ETubeDirection>({ TL, BR, BR, XX })));

	// Shorten
	CTube tubeShorten(50, 50, TC, 47, 47, std::vector<ETubeDirection>({ TL, TL, TL, XX }));
	tubeShorten.append(48, 48);
	REPORT_TEST(tubeShorten == CTube(50, 50, TC, 48, 48, std::vector<ETubeDirection>({ TL, TL, XX })));

}

void Tests::test_tube_delimiter()
{
	using namespace TubeDirections;

	// delimiter needs to be auto-added in any case as it causes crashes in TS/RA2 if it's missing somehow
	//REPORT_TEST(CTube(50, 50, XX, 50, 50, std::vector<ETubeDirection>()) == CTube(50, 50, XX, 50, 50, std::vector<ETubeDirection>({ XX })));
	//REPORT_TEST(CTube(50, 50, XX, 49, 49, std::vector<ETubeDirection>({ TL })) == CTube(50, 50, XX, 49, 49, std::vector<ETubeDirection>({ TL, XX })));
	REPORT_TEST(CTube(1, "50, 50, -1, 50, 50").toString() == "50,50,-1,50,50,-1");

	// right now we're also fixing when loading the map, TBD:
	REPORT_TEST(CTube(0xFFFF, "50, 50, -1, 50, 50") == CTube(50, 50, XX, 50, 50, std::vector<ETubeDirection>({ XX })));
}

void Tests::test_hsv()
{
	typedef std::array<unsigned char, 3> ba;
	REPORT_TEST(HSVToRGB(0, 1.0, 1.0) == ba({ 255, 0, 0 }));
	REPORT_TEST(HSVToRGB(0, 0.0, 1.0) == ba({ 255, 255, 255 }));
	REPORT_TEST(HSVToRGB(0, 0.0, 0.0) == ba({ 0, 0, 0 }));
	REPORT_TEST(HSVToRGB(45.0, 1.0, 1.0) == ba({ 255, 191, 0 }));
	REPORT_TEST(HSVToRGB(75.0, 1.0, 1.0) == ba({ 191, 255, 0 }));
	REPORT_TEST(HSVToRGB(135.0, 1.0, 1.0) == ba({ 0, 255, 63 }));
	REPORT_TEST(HSVToRGB(240.0, 1.0, 1.0) == ba({ 0, 0, 255 }));
	REPORT_TEST(HSVToRGB(184.0, 1.0, 1.0) == ba({ 0, 238, 255 }));
	REPORT_TEST(HSVToRGB(285.0, 1.0, 1.0) == ba({ 191, 0, 255 }));
	REPORT_TEST(HSVToRGB(330.0, 1.0, 1.0) == ba({ 255, 0, 127 }));
	REPORT_TEST(HSVToRGB(180.0, 1.0, 0.5) == ba({ 0, 127, 127 }));
	REPORT_TEST(HSVToRGB(180.0, 0.5, 0.5) == ba({ 63, 127, 127 }));
}

void Tests::test_iso()
{
	CMapData d;
	d.CreateMap(16, 10, 0, 0);
	REPORT_TEST(d.ProjectCoords3d(MapCoords(0, 0)) == ProjectedCoords((26 - 2) * f_x / 2, 0));
	REPORT_TEST(d.ProjectCoords3d(MapCoords(1, 0)) == ProjectedCoords((26 - 2 - 1) * f_x / 2, f_y / 2));
	REPORT_TEST(d.ProjectCoords3d(MapCoords(0, 1)) == ProjectedCoords((26 - 2 + 1) * f_x / 2, f_y / 2));
	REPORT_TEST(d.ProjectCoords3d(MapCoords(1, 1)) == ProjectedCoords((26 - 2) * f_x / 2, f_y));
	REPORT_TEST(d.ProjectCoords3d(MapCoords(1, 1), 1) == ProjectedCoords((26 - 2) * f_x / 2, f_y / 2));

	REPORT_TEST(d.ToMapCoords3d(ProjectedCoords((26 - 2) * f_x / 2, 0), 0) == MapCoords(0, 0));
	
}
