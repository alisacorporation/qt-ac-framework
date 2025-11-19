cmake_minimum_required(VERSION 3.16)

project(qt_slr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appqt_slr
    main.cpp
    SystemMonitor.h
    SystemMonitor.cpp
    DiskAnalyzer.h
    DiskAnalyzer.cpp
    ServerManager.h
    ServerManager.cpp
)

qt_add_qml_module(appqt_slr
    URI qt_slr
    QML_FILES
        Main.qml
        StatCircle.qml
        NetworkCircle.qml
        Sidebar.qml
        DiskCleanup.qml
        RemoteMonitor.qml
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
