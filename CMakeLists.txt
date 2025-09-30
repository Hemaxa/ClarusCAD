#определение минимальной версии CMake
cmake_minimum_required(VERSION 3.19)

#определение версии проекта
project(ClarusCAD VERSION 1.0 LANGUAGES CXX)

#определение версии C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#определение используемых пакетов
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Svg)

#включение Qt-автоматизаций
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#создание переменных для src и res директорий
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res)

#создание исполняемого файла
add_executable(ClarusCAD)

#обозначение исходных файлов
target_sources (ClarusCAD PRIVATE
    ${SRC_DIR}/Main.cpp

    #controller
    ${SRC_DIR}/controller/BaseCreationTool.h
    ${SRC_DIR}/controller/CommandParser.h
    ${SRC_DIR}/controller/CommandParser.cpp

    ${SRC_DIR}/controller/tools/DeleteTool.h
    ${SRC_DIR}/controller/tools/DeleteTool.cpp
    ${SRC_DIR}/controller/tools/SegmentCreationTool.h
    ${SRC_DIR}/controller/tools/SegmentCreationTool.cpp

    #model
    ${SRC_DIR}/model/BasePrimitive.h
    ${SRC_DIR}/model/EnumManager.h
    ${SRC_DIR}/model/Scene.h
    ${SRC_DIR}/model/Scene.cpp

    ${SRC_DIR}/model/primitives/PointPrimitive.h
    ${SRC_DIR}/model/primitives/PointPrimitive.cpp
    ${SRC_DIR}/model/primitives/SegmentPrimitive.h
    ${SRC_DIR}/model/primitives/SegmentPrimitive.cpp

    #view
    ${SRC_DIR}/view/BasePanelWidget.h
    ${SRC_DIR}/view/BaseDrawingTool.h
    ${SRC_DIR}/view/BasePropertiesWidget.h
    ${SRC_DIR}/view/BasePropertiesWidget.cpp
    ${SRC_DIR}/view/MainWindow.h
    ${SRC_DIR}/view/MainWindow.cpp
    ${SRC_DIR}/view/SettingsWindow.h
    ${SRC_DIR}/view/SettingsWindow.cpp

    ${SRC_DIR}/view/design/AnimationManager.h
    ${SRC_DIR}/view/design/AnimationManager.cpp
    ${SRC_DIR}/view/design/ThemeManager.h
    ${SRC_DIR}/view/design/ThemeManager.cpp

    ${SRC_DIR}/view/drawing/SegmentDrawingTool.h
    ${SRC_DIR}/view/drawing/SegmentDrawingTool.cpp

    ${SRC_DIR}/view/panels/ConsolePanelWidget.h
    ${SRC_DIR}/view/panels/ConsolePanelWidget.cpp
    ${SRC_DIR}/view/panels/PropertiesPanelWidget.h
    ${SRC_DIR}/view/panels/PropertiesPanelWidget.cpp
    ${SRC_DIR}/view/panels/SceneObjectsPanelWidget.h
    ${SRC_DIR}/view/panels/SceneObjectsPanelWidget.cpp
    ${SRC_DIR}/view/panels/SceneSettingsPanelWidget.h
    ${SRC_DIR}/view/panels/SceneSettingsPanelWidget.cpp
    ${SRC_DIR}/view/panels/ToolbarPanelWidget.h
    ${SRC_DIR}/view/panels/ToolbarPanelWidget.cpp
    ${SRC_DIR}/view/panels/ViewportPanelWidget.h
    ${SRC_DIR}/view/panels/ViewportPanelWidget.cpp

    ${SRC_DIR}/view/properties/SegmentPropertiesWidget.h
    ${SRC_DIR}/view/properties/SegmentPropertiesWidget.cpp

    #resources
    ${RES_DIR}/resources.qrc
)

#директории с заголовочными файлами
target_include_directories(ClarusCAD PRIVATE
    #controller
    ${SRC_DIR}/controller
    ${SRC_DIR}/controller/tools

    #model
    ${SRC_DIR}/model
    ${SRC_DIR}/model/primitives

    #view
    ${SRC_DIR}/view
    ${SRC_DIR}/view/drawing
    ${SRC_DIR}/view/design
    ${SRC_DIR}/view/panels
    ${SRC_DIR}/view/properties
)

#подключение необходимых библиотек
target_link_libraries(ClarusCAD PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Svg
)

include(GNUInstallDirs)

install(TARGETS ClarusCAD
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)





#CI/CD
set(CPACK_PACKAGE_NAME "ClarusCAD")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Sergey Yashin")
set(CPACK_PACKAGE_CONTACT "your.email@example.com")

if(WIN32)
    set_target_properties(ClarusCAD PROPERTIES WIN32_EXECUTABLE TRUE)
    target_sources(ClarusCAD PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/packs/resources.rc")
endif()

if(APPLE)
    set_target_properties(ClarusCAD PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packs/Info.plist.in"
    )
    add_custom_command(TARGET ClarusCAD POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/res/app-icon/icon.icns"
        "$<TARGET_BUNDLE_DIR:ClarusCAD>/Contents/Resources/icon.icns"
    )
endif()

if(UNIX AND NOT APPLE)
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/packs/linux/Info.desktop.in"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
        RENAME "ClarusCAD.png"
    )
endif()
