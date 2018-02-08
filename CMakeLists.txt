#**************************************************************************
#   Copyright (C) 2017 by:                                                *
#   Vanderson Rosario (vandersonmr2@gmail.com)                            *
#                                                                         *
#   This program is free software; you can redistribute it and/or modify  *
#   it under the terms of the GNU General Public License as published by  *
#   the Free Software Foundation; either version 3 of the License, or     *
#   (at your option) any later version.                                   *
#                                                                         *
#   This program is distributed in the hope that it will be useful,       *
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#   GNU General Public License for more details.                          *
#                                                                         *
#   You should have received a copy of the GNU General Public License     *
#   along with this program; if not, write to the                         *
#   Free Software Foundation, Inc.,                                       *
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
#**************************************************************************
cmake_minimum_required (VERSION 2.6)

project (RAIn3)

set(CMAKE_CXX_FLAGS "-g -Werror -Wall -std=c++17 -fopenmp -O0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

#set(CMAKE_CXX_FLAGS "-O0 -Werror -Wall -std=c++17 -fno-rtti -fmax-errors=5 -g")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0")

include_directories ("${PROJECT_SOURCE_DIR}/include/")

# Add libraries
add_subdirectory(src)

add_executable(rain3 main.cpp)
add_executable(genhist generateHist.cpp)
add_executable(gencfg generateDCFG.cpp)

target_link_libraries (rain3 rainlib tracelib arglib)
target_link_libraries (genhist rainlib tracelib arglib)
target_link_libraries (gencfg rainlib tracelib arglib)

INSTALL(TARGETS rain3 genhist RUNTIME DESTINATION bin)
