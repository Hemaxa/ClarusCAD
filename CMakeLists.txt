cmake_minimum_required(VERSION 3.19)
project(ClarusCAD LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_add_executable(ClarusCAD
    WIN32 MACOSX_BUNDLE
    src/Main.cpp
    src/view/MainWindow.cpp
    src/view/MainWindow.h
    src/view/ViewportWidget.h
    src/view/ViewportWidget.cpp
)

target_link_libraries(ClarusCAD
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS ClarusCAD
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ClarusCAD
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
