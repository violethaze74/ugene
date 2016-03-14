cmake_minimum_required(VERSION 3.4)
project(ugeneui)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5 REQUIRED Core Gui Widgets Xml Network Script Svg WebKit WebKitWidgets)

add_definitions(-DHI_EXCLUDED)

include_directories(src)
include_directories(../include)
include_directories(../corelibs/U2Private/src)
include_directories(../libs_3rdparty/QSpec/src)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

qt5_add_resources(RCC_SRCS src/ugeneui.qrc)

add_executable(ugeneui ${HDRS} ${SRCS})

target_link_libraries(ugeneui
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Script Qt5::WebKit Qt5::WebKitWidgets
        gtest ugenedb breakpad samtools
        U2Core U2Algorithm U2Formats U2Remote U2Lang U2Gui U2View U2Test)

