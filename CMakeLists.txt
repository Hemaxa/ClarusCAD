#определение минимальной версии CMake
cmake_minimum_required(VERSION 3.19)

#определение версии проекта
project(ClarusCAD VERSION 1.0 LANGUAGES CXX)

#определение используемых пакетов
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)

qt_standard_project_setup()

#создание переменной для src директории
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#обозначение исходных файлов
qt_add_executable (ClarusCAD WIN32 MACOSX_BUNDLE
    ${SRC_DIR}/Main.cpp

    #view
    ${SRC_DIR}/view/MainWindow.h
    ${SRC_DIR}/view/MainWindow.cpp
    ${SRC_DIR}/view/ViewportWidget.h
    ${SRC_DIR}/view/ViewportWidget.cpp
    ${SRC_DIR}/view/LinePropertiesWidget.h
    ${SRC_DIR}/view/LinePropertiesWidget.cpp
    ${SRC_DIR}/view/ToolbarPanel.h
    ${SRC_DIR}/view/ToolbarPanel.cpp

    #model
    ${SRC_DIR}/model/Scene.h
    ${SRC_DIR}/model/Scene.cpp
    ${SRC_DIR}/model/BasePrimitive.h
    ${SRC_DIR}/model/primitives/Point.h
    ${SRC_DIR}/model/primitives/Point.cpp
    ${SRC_DIR}/model/primitives/Segment.h
    ${SRC_DIR}/model/primitives/Segment.cpp

    #controller
    ${SRC_DIR}/controller/BaseTool.h
    ${SRC_DIR}/controller/tools/LineCreationTool.h
    ${SRC_DIR}/controller/tools/LineCreationTool.cpp
)

#директории с заголовочными файлами
target_include_directories(ClarusCAD PRIVATE
    ${SRC_DIR}/controller
    ${SRC_DIR}/controller/tools
    ${SRC_DIR}/model
    ${SRC_DIR}/model/primitives
    ${SRC_DIR}/view
)

#подключение необходимых библиотек
target_link_libraries(ClarusCAD PRIVATE
    Qt::Core
    Qt::Gui
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
