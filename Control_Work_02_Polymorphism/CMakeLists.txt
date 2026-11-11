cmake_minimum_required(VERSION 3.16)
project(Control_Work_02_Polymorphism LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(polymorphism_model
    uav.h uav.cpp sensor.h sensor.cpp navigation.h navigation.cpp
    controller.h controller.cpp autopilot.h autopilot.cpp point3d.h)
target_include_directories(polymorphism_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(polymorphism_model PUBLIC Qt${QT_VERSION_MAJOR}::Core)
if(MSVC)
    target_compile_options(polymorphism_model PUBLIC /utf-8)
endif()

add_executable(polymorphism_demo demo.cpp)
target_link_libraries(polymorphism_demo PRIVATE polymorphism_model)

option(KR2_BUILD_GUI "Build Qt Widgets / Qt3D / Multimedia visualization" ON)
if(KR2_BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets 3DCore 3DRender 3DExtras 3DInput Multimedia Svg)
    add_library(polymorphism_gui STATIC
        mainwindow.h mainwindow.cpp scene3d.h scene3d.cpp dronevisual.h dronevisual.cpp
        signalgraph.h signalgraph.cpp flightaudio.h flightaudio.cpp)
    target_link_libraries(polymorphism_gui PUBLIC polymorphism_model
        Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::3DCore
        Qt${QT_VERSION_MAJOR}::3DRender Qt${QT_VERSION_MAJOR}::3DExtras
        Qt${QT_VERSION_MAJOR}::3DInput Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Svg)
    add_executable(Control_Work_02_Polymorphism main.cpp assets.qrc)
    target_link_libraries(Control_Work_02_Polymorphism PRIVATE polymorphism_gui)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(polymorphism_tests tests.cpp)
    target_link_libraries(polymorphism_tests PRIVATE polymorphism_model)
    add_test(NAME polymorphism_model COMMAND polymorphism_tests)
    if(KR2_BUILD_GUI)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
        add_executable(polymorphism_window_tests window_tests.cpp assets.qrc)
        target_link_libraries(polymorphism_window_tests PRIVATE polymorphism_gui Qt${QT_VERSION_MAJOR}::Test)
        add_test(NAME polymorphism_window COMMAND polymorphism_window_tests)
        set_tests_properties(polymorphism_window PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
    endif()
endif()
