cmake_minimum_required(VERSION 3.5)

project(server-gui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER /usr/bin/clang-11)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-11)
set(QT_QMAKE_EXECUTABLE /usr/lib/qt5/bin/qmake)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES server-gui_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        servergui.cpp
        servergui.h
        servergui.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(server-gui
        ${PROJECT_SOURCES}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(server-gui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(server-gui
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(server-gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
