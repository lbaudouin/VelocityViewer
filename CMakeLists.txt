# Copyright 2014, Léo Baudouin
# leo.baudouin@univ-bpclermont.fr
#
# This file is part of a free library: you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details. You should have
# received a copy of the GNU General Public License along with
# this library. If not, see <http://www.gnu.org/licenses/>.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#Define the project
PROJECT(VelocityViewer)

SET(PROJECT_DESCRIPTION "")
SET(PROJECT_URL "")
SET(PROJECT_VERSION_MAJOR 0)
SET(PROJECT_VERSION_MINOR 0)
SET(PROJECT_VERSION_PATCH 1)
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
SET(PROJECT_REQUIREMENTS "")

#SET(CUSTOM_HEADER_DIR "${PROJECT_NAME}")

#Set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

#Set CXX parameters
SET(CXX_DISABLE_WERROR 1)

#Options
OPTION(BUILD_SHARED_LIBS "Build the shared library." TRUE)
OPTION(BUILD_TEST "Build test" FALSE)
OPTION(INSTALL_DOC "Install documentation." FALSE)
OPTION(INSTALL_BINARIES "Install binaries." FALSE)

#Find packages
FIND_PACKAGE( Qt4 4.8 COMPONENTS QTCORE QTGUI REQUIRED)
INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIRS} )
INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )
#SET( CMAKE_AUTOMOC ON )

FIND_PACKAGE(Boost 1.46 COMPONENTS thread system REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

SET( ${PROJECT_NAME}_LIBS ${QT_LIBRARIES} ${Boost_LIBRARIES} )

#Include directories
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include/ )
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )

#Set sources files
FILE(GLOB ${PROJECT_NAME}_SRCS  ${CMAKE_SOURCE_DIR}/src/*.cpp)

#Set headers files
FILE(GLOB ${PROJECT_NAME}_HDRS  ${CMAKE_SOURCE_DIR}/include/*.h)

#Create UI
FILE(GLOB ${PROJECT_NAME}_UI  ${CMAKE_SOURCE_DIR}/ui/*.ui)
QT4_WRAP_UI( UI_H ${${PROJECT_NAME}_UI} )

#Create ressources
FILE(GLOB ${PROJECT_NAME}_RCC  ${CMAKE_SOURCE_DIR}/ressources/*.qrc)
QT4_ADD_RESOURCES( RCC_FILES ${${PROJECT_NAME}_RCC} )

#Create MOC
QT4_WRAP_CPP( MOC_FILES ${${PROJECT_NAME}_HDRS} )

#Build library
#Library type (static|shared) depend on BUILD_SHARED_LIBS variable
ADD_LIBRARY ( ${PROJECT_NAME} ${UI_H} ${MOC_FILES} ${RCC_FILES} ${${PROJECT_NAME}_SRCS} )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${${PROJECT_NAME}_LIBS} )
LINK_DIRECTORIES( ${PROJECT_BINARY_DIR} )

SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
     SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
     VERSION ${PROJECT_VERSION})

#Build binaries
IF(EXISTS ${CMAKE_SOURCE_DIR}/bin/CMakeLists.txt)
  ADD_SUBDIRECTORY(bin)
ENDIF()

#Build tests
IF(EXISTS ${CMAKE_SOURCE_DIR}/samples/CMakeLists.txt)
  ADD_SUBDIRECTORY(samples)
ENDIF()

#Build unit testing
IF(BUILD_TEST)
  ENABLE_TESTING()
  IF(EXISTS ${CMAKE_SOURCE_DIR}/unitTesting/CMakeLists.txt)
    ADD_SUBDIRECTORY(unitTesting)
  ENDIF()
ENDIF(BUILD_TEST)

#Install headers
INSTALL(FILES ${${PROJECT_NAME}_HDRS}
	DESTINATION include/${PROJECT_NAME}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)

#Install library
INSTALL(TARGETS ${PROJECT_NAME}
	DESTINATION lib
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
        
#Uninstall library
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
 

#Configure and install LibraryConfig.cmake.in
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/LibraryConfig.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" )

INSTALL(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	DESTINATION share/${PROJECT_NAME}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)

#Configure and install LibraryConfigVersion.cmake.in
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/LibraryConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

INSTALL(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	DESTINATION share/${PROJECT_NAME}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)

#Configure and install Library.pc.cmake
SET(install_pkg_prefix "\${prefix}")
SET(install_pkg_exec_prefix "\${exec_prefix}")
SET(install_pkg_libdir "\${libdir}")
SET(install_pkg_include_dir "\${includedir}")
SET(install_libdir_pkgconfig ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Library.pc.in
		 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc )

INSTALL(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc
	DESTINATION ${install_libdir_pkgconfig}	
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

#Create documentation
FIND_PACKAGE( Doxygen )

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile )
  #Add doc target
  ADD_CUSTOM_TARGET(doc
  ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
  COMMENT "Generating API documentation with Doxygen" VERBATIM)
  IF(INSTALL_DOC)
    #Force build doc before install
    INSTALL( CODE "EXEC_PROGRAM(${CMAKE_BUILD_TOOL} ARGS doc)" )
    #Install documentation
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html/
	    DESTINATION share/doc/${PROJECT_NAME})
  ENDIF(INSTALL_DOC)
ENDIF(DOXYGEN_FOUND)
