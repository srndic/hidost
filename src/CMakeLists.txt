# Copyright 2014 Nedim Srndic, University of Tuebingen
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

# Build and install the Hidost executables

# A macro for aborting if a required library is not found
macro(require_library)
    foreach(MYLIB ${ARGN})
        find_library(RL_LIB ${MYLIB})
        if (RL_LIB STREQUAL "RL_LIB-NOTFOUND")
                message(FATAL_ERROR "Missing library '${MYLIB}'")
        else (RL_LIB STREQUAL "RL_LIB-NOTFOUND")
                message(STATUS "Found library '${MYLIB}'")
        endif (RL_LIB STREQUAL "RL_LIB-NOTFOUND")
        unset_full(RL_LIB)
    endforeach()
endmacro(require_library)

# Some source files need configuring
configure_file(cacher.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/cacher.cpp)
configure_file(pathcount.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/pathcount.cpp)

if (CACHER)
    set(REQUIRED_LIBS quickly boost_program_options boost_thread boost_filesystem boost_system)
    require_library(${REQUIRED_LIBS})
	set(CACHER_SOURCES cacher.cpp)
	add_executable(${CACHER_EXECUTABLE_NAME} ${CACHER_SOURCES})
	target_link_libraries(${CACHER_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${CACHER_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${CACHER_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (CACHER)

if (FEATEXTRACT)
    set(REQUIRED_LIBS quickly boost_program_options boost_thread boost_system)
    require_library(${REQUIRED_LIBS})
	set(FEATEXTRACT_SOURCES NPPFFile.cpp pdfpath.cpp feat-extract.cpp)
	add_executable(${FEATEXTRACT_EXECUTABLE_NAME} ${FEATEXTRACT_SOURCES})
	target_link_libraries(${FEATEXTRACT_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${FEATEXTRACT_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${FEATEXTRACT_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (FEATEXTRACT)

if (FEATSELECT)
    set(REQUIRED_LIBS boost_program_options)
    require_library(${REQUIRED_LIBS})
	set(FEATSELECT_SOURCES pdfpath.cpp feat-select.cpp)
	add_executable(${FEATSELECT_EXECUTABLE_NAME} ${FEATSELECT_SOURCES})
	target_link_libraries(${FEATSELECT_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${FEATSELECT_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${FEATSELECT_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (FEATSELECT)

if (MERGER)
	set(MERGER_SOURCES pdfpath.cpp merger.cpp)
	add_executable(${MERGER_EXECUTABLE_NAME} ${MERGER_SOURCES})
	set_target_properties(${MERGER_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${MERGER_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (MERGER)

if (PATHCOUNT)
    set(REQUIRED_LIBS quickly boost_program_options boost_thread boost_system)
    require_library(${REQUIRED_LIBS})
	set(PATHCOUNT_SOURCES pathcount.cpp)
	add_executable(${PATHCOUNT_EXECUTABLE_NAME} ${PATHCOUNT_SOURCES})
	target_link_libraries(${PATHCOUNT_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${PATHCOUNT_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${PATHCOUNT_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (PATHCOUNT)

if (PDF2PATHS)
    set(REQUIRED_LIBS poppler boost_regex)
    require_library(${REQUIRED_LIBS})
   	set(PDF2PATHS_SOURCES pdfpath.cpp pdf2paths.cpp)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/poppler")
	add_executable(${PDF2PATHS_EXECUTABLE_NAME} ${PDF2PATHS_SOURCES})
	target_link_libraries(${PDF2PATHS_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${PDF2PATHS_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${PDF2PATHS_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (PDF2PATHS)

if (PDF2VALS)
    set(REQUIRED_LIBS poppler boost_regex)
    require_library(${REQUIRED_LIBS})
	set(PDF2VALS_SOURCES pdfpath.cpp pdf2vals.cpp)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/poppler")
	add_executable(${PDF2VALS_EXECUTABLE_NAME} ${PDF2VALS_SOURCES})
	target_link_libraries(${PDF2VALS_EXECUTABLE_NAME} ${REQUIRED_LIBS})
	set_target_properties(${PDF2VALS_EXECUTABLE_NAME} PROPERTIES VERSION ${HIDOST_VERSION})
	install(TARGETS ${PDF2VALS_EXECUTABLE_NAME}
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (PDF2VALS)
