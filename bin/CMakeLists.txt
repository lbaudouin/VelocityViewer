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

#VelocityViewer-demo
ADD_EXECUTABLE(velocityviewer-demo
	velocityviewer-demo.cpp
)
TARGET_LINK_LIBRARIES( velocityviewer-demo ${PROJECT_NAME} ${QT_LIBRARIES} ${Boost_LIBRARIES}  )
MESSAGE(STATUS ${${PROJECT_NAME}_LIBS})

#Install binaries
IF(INSTALL_BINARIES)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/velocityviewer-demo
	DESTINATION bin
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE OWNER_EXECUTE)
ENDIF(INSTALL_BINARIES)
