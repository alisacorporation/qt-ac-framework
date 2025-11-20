cmake_minimum_required(VERSION 3.16)

project(qt_slr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

qt_add_executable(appqt_slr
    src/main.cpp
    include/SystemMonitor.h
    src/core/SystemMonitor.cpp
    include/DiskAnalyzer.h
    src/core/DiskAnalyzer.cpp
    include/ServerManager.h
    src/core/ServerManager.cpp
)

qt_add_qml_module(appqt_slr
    URI qt_slr
    QML_FILES
        src/ui/views/Main.qml
        src/ui/views/DiskCleanup.qml
        src/ui/views/RemoteMonitor.qml
        src/ui/components/StatCircle.qml
        src/ui/components/NetworkCircle.qml
        src/ui/components/SparkLine.qml
        src/ui/components/Sidebar.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appqt_slr PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqt_slr
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqt_slr
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appqt_slr
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
