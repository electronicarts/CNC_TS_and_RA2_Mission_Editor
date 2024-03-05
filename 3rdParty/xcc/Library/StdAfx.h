/*
    XCC Utilities and Library
    Copyright (C) 2000  Olaf van der Spek  <olafvdspek@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

// #define WIN32_LEAN_AND_MEAN

#include <array>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <climits>
#include <cstdint>
#include <list>
#include <memory>
#include <map>
#include <set>
#include <span>
#include <string>
#include <vartypes.h>
#include <vector>
#include <xcc/data_ref.h>
#include <xcc/find_ptr.h>
#ifndef XCC_MINIMAL_BUILD
#include <xcc/string_view.h>
#endif

using namespace std;
using boost::iequals;
using boost::to_lower_copy;
using boost::trim_copy;
