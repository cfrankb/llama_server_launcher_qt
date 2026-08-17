cmake_minimum_required(VERSION 3.16)
project(LlamaServerLauncher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Gui)

set(SOURCES
    main.cpp
    MainWindow.cpp
    ConfigManager.cpp
    ServerProcess.cpp
)

set(HEADERS
    MainWindow.h
    ConfigManager.h
    ServerProcess.h
)

add_executable(LlamaServerLauncher
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(LlamaServerLauncher
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
)
