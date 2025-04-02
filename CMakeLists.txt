# Copyright (C) 2015-2025 maClara, LLC <info@maclara-llc.com>
# This file is part of libpex87xx
#
# SPDX-License-Identifier:  MPL-2.0
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required (VERSION 3.7...3.15)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Versions)

project(${PEX87XX_PROJECT}
	VERSION ${PEX87XX_VERSION}
	DESCRIPTION ${PEX87XX_DESCRIPTION}
	HOMEPAGE_URL ${PEX87XX_HOMEPAGE_URL}
	LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -O2")

# Must be set after the above
include(GNUInstallDirs)

add_library(pex87xx SHARED)
add_library(pex87xx_static STATIC)
set_target_properties(pex87xx_static PROPERTIES
	OUTPUT_NAME pex87xx)

set(PEX87XX_SOURCES src/pex87xx.c)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR})

target_sources(pex87xx PRIVATE ${PEX87XX_SOURCES})
target_sources(pex87xx_static PRIVATE ${PEX87XX_SOURCES})

set(EXAMPLES)

function(pex87xx_add_example)
        set(oneValueArgs NAME SRC DIR)
	cmake_parse_arguments(Example "" "${oneValueArgs}" "" ${ARGN})

	list(APPEND EXAMPLES ${Example_NAME})
	add_executable(${Example_NAME} ${Example_SRC})
	target_link_libraries(${Example_NAME} PRIVATE pex87xx_static)
	set_target_properties(${Example_NAME} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY
                "${CMAKE_BINARY_DIR}/examples")
endfunction()

pex87xx_add_example(NAME pex-status SRC examples/pex-status.c)
pex87xx_add_example(NAME pex-dump SRC examples/pex-dump.c)
pex87xx_add_example(NAME pex-write SRC examples/pex-write.c)
pex87xx_add_example(NAME pex-probe SRC examples/pex-probe.c)

target_include_directories(pex87xx PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

set_target_properties(pex87xx PROPERTIES
	VERSION ${PEX87XX_VERSION_INFO}
	SOVERSION ${PEX87XX_COMPATVERSION}
)

add_definitions(-D_GNU_SOURCE)

# Install header
install(FILES src/pex87xx.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE README.md
	DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Install library
install(TARGETS pex87xx
	EXPORT ${PEX87XX_PROJECT}Targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install Static library
install(TARGETS pex87xx_static
        EXPORT ${PEX87XX_PROJECT}StaticTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
