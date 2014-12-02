# Copyright 2012 Nedim Srndic, University of Tuebingen
# 
# This file is part of Hidost.
#
# Hidost is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Hidost is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Hidost.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)
project(hidost)
set(HIDOST_VERSION "1.0")

# A macro to unset names
macro(unset_full)
    foreach(MYVAR ${ARGN})
        unset(${MYVAR} CACHE)
        unset(${MYVAR})
    endforeach()
endmacro(unset_full)

# Unset all names
unset_full(CACHER CUMULATIVE FEAT_EXTRACT FEAT_EXTRACTVALS FEAT_SELECT MERGER PATHCOUNT PDF2PATHS PDF2VALS)


# Set executable names
set(CACHER_EXECUTABLE_NAME cacher)
set(CUMULATIVE_EXECUTABLE_NAME cumulative)
set(FEATEXTRACT_EXECUTABLE_NAME feat-extract)
set(FEATEXTRACTVALS_EXECUTABLE_NAME feat-extract-vals)
set(FEATSELECT_EXECUTABLE_NAME feat-select)
set(MERGER_EXECUTABLE_NAME merger)
set(PATHCOUNT_EXECUTABLE_NAME pathcount)
set(PDF2PATHS_EXECUTABLE_NAME pdf2paths)
set(PDF2VALS_EXECUTABLE_NAME pdf2vals)

# Make sure the tools to be built are all defined
# Run with -DTOOLSET='tool1;tool2' to select individual tools
if (TOOLSET)
	foreach(TOOL ${TOOLSET})
	    if (TOOL STREQUAL ${CACHER_EXECUTABLE_NAME})
	        set(CACHER 1)
	        set(PDF2PATHS 1) # required for cacher
	    elseif (TOOL STREQUAL ${CUMULATIVE_EXECUTABLE_NAME})
	        set(CUMULATIVE 1)
	    elseif (TOOL STREQUAL ${FEATEXTRACT_EXECUTABLE_NAME})
	        set(FEATEXTRACT 1)
	    elseif (TOOL STREQUAL ${FEATEXTRACTVALS_EXECUTABLE_NAME})
	        set(FEATEXTRACTVALS 1)
        elseif (TOOL STREQUAL ${FEATSELECT_EXECUTABLE_NAME})
	        set(FEATSELECT 1)
	    elseif (TOOL STREQUAL ${MERGER_EXECUTABLE_NAME})
	        set(MERGER 1)
	    elseif (TOOL STREQUAL ${PATHCOUNT_EXECUTABLE_NAME})
	        set(PATHCOUNT 1)
	        set(MERGER 1) # required for pathcount
	    elseif (TOOL STREQUAL ${PDF2PATHS_EXECUTABLE_NAME})
			set(PDF2PATHS 1)
		elseif (TOOL STREQUAL ${PDF2VALS_EXECUTABLE_NAME})
			set(PDF2VALS 1)
		else (TOOL STREQUAL ${CACHER_EXECUTABLE_NAME})
			message(FATAL_ERROR "Unknown tool '${TOOL}'")
		endif (TOOL STREQUAL ${CACHER_EXECUTABLE_NAME})
		message(STATUS "Preparing to build tool '${TOOL}'")
	endforeach(TOOL)
	unset_full(TOOLSET)
else (TOOLSET)
	message(STATUS "Toolset not defined. Will make all tools.")
	set(CACHER 1)
	set(CUMULATIVE 1)
	set(FEATEXTRACT 1)
	set(FEATEXTRACTVALS 1)
	set(FEATSELECT 1)
	set(MERGER 1)
	set(PATHCOUNT 1)
	set(PDF2PATHS 1)
	set(PDF2VALS 1)
endif (TOOLSET)

# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wformat=2 -Wall -Wextra -std=c++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

# Source directory
add_subdirectory(src)
