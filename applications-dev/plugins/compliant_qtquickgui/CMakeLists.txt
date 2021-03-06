cmake_minimum_required(VERSION 2.8.12)
project(compliant_qtquickgui)

set(compliant_qtquickgui_MAJOR_VERSION 0)
set(compliant_qtquickgui_MINOR_VERSION 1)
set(compliant_qtquickgui_VERSION ${compliant_qtquickgui_MAJOR_VERSION}.${compliant_qtquickgui_MINOR_VERSION})

set(SOURCE_FILES
  CompliantQtQuickGUI.cpp
  SofaCompliantInteractor.cpp
)

set(MOC_FILES
    SofaCompliantInteractor.h
)

set(HEADER_FILES
  CompliantQtQuickGUI.h  
  SofaCompliantInteractor.h
)

set(QML_FILES
    data/qml/SofaInteractors/UserInteractor_Compliant.qml
)

set(QRC_FILES
    data/qml/compliant_qml.qrc
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(SofaFramework REQUIRED)
find_package(Compliant REQUIRED)
find_package(SofaQtQuickGUI REQUIRED)

# TODO do we need all the crap following?


# on Window, Qt packages need the glu32 lib full path, but they do not find it without help
if(WIN32)
    if(CMAKE_CL_64)
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64")
    else()
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x86")
    endif()

    # to fix a bug when one is compiling a debug version of the code
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DQT_NO_DEBUG)
    endif()
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Quick Qml REQUIRED PATHS "${SOFA-EXTERNAL_QT5_PATH}")
# make sure to use QT > 5.0

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Qml_INCLUDE_DIRS})
include_directories(${Qt5Quick_INCLUDE_DIRS})

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Qml_DEFINITIONS})
add_definitions(${Qt5Quick_DEFINITIONS})


qt5_wrap_cpp(MOC_FILES ${MOC_FILES}) # do not use "set(CMAKE_AUTOMOC ON)" since all the mocced files will be compiled by a single compilation unit leading to a "out of heap space" issue on MSVC
qt5_add_resources(RESOURCE_FILES ${QRC_FILES})

add_definitions(-DQT_PLUGIN)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${MOC_FILES} ${SOURCE_FILES} ${QRC_FILES} ${RESOURCE_FILES} ${QML_FILES} ${CONFIG_FILES})


# wtf is this
if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:${CXX11_FLAG}>")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-DSOFA_BUILD_COMPLIANT_QTQUICKGUI")

target_link_libraries(${PROJECT_NAME} Compliant SofaQtQuickGUI)

## Install rules for the library; CMake package configurations files
sofa_create_package(${PROJECT_NAME} ${compliant_qtquickgui_VERSION} ${PROJECT_NAME} ${PROJECT_NAME})
