cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(sopcast_player)

# Qt options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find Qt5 flags
find_package(Qt5Widgets)
include_directories(${Qt5Widgets_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Qt5Widgets_LIBRARIES})

find_package(Qt5Network)
include_directories(${Qt5Network_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Qt5Network_LIBRARIES})

# compile
file(GLOB src_files *.cpp *.h *.ui *.qrc)
add_executable(sopcast_player ${src_files})
target_link_libraries(sopcast_player ${LIBS})
