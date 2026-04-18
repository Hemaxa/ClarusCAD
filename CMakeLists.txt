#определение минимальной версии CMake
cmake_minimum_required(VERSION 3.21.1)

#определение версии проекта
project(ClarusCAD VERSION 1.0 LANGUAGES CXX)

#определение стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#определение используемых пакетов
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

#включение Qt-автоматизаций
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#создание переменных для src и res директорий
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res)

#создание исполняемого файла
add_executable(ClarusCAD
    src/model/primitives/RectanglePrimitive.h src/model/primitives/RectanglePrimitive.cpp
    src/model/primitives/ArcPrimitive.h src/model/primitives/ArcPrimitive.cpp
    src/view/drawing/RectangleDrawingTool.h src/view/drawing/RectangleDrawingTool.cpp
    src/view/drawing/ArcDrawingTool.h src/view/drawing/ArcDrawingTool.cpp
    src/controller/tools/RectangleCreationTool.h src/controller/tools/RectangleCreationTool.cpp
    src/controller/tools/ArcCreationTool.h src/controller/tools/ArcCreationTool.cpp
    src/view/properties/RectanglePropertiesWidget.h src/view/properties/RectanglePropertiesWidget.cpp
    src/view/properties/ArcPropertiesWidget.h src/view/properties/ArcPropertiesWidget.cpp
    src/model/primitives/EllipsePrimitive.h src/model/primitives/EllipsePrimitive.cpp
    src/view/drawing/EllipseDrawingTool.h src/view/drawing/EllipseDrawingTool.cpp
    src/controller/tools/EllipseCreationTool.h src/controller/tools/EllipseCreationTool.cpp
    src/view/properties/EllipsePropertiesWidget.h src/view/properties/EllipsePropertiesWidget.cpp
    src/view/properties/DimensionPropertiesWidget.h src/view/properties/DimensionPropertiesWidget.cpp
    src/view/managers/ObjectBindingManager.h src/view/managers/ObjectBindingManager.cpp)

#обозначение исходных файлов
target_sources (ClarusCAD PRIVATE
    ${SRC_DIR}/Main.cpp

    #controller
    ${SRC_DIR}/controller/BaseCreationTool.h
    ${SRC_DIR}/controller/CommandParser.h
    ${SRC_DIR}/controller/CommandParser.cpp

    ${SRC_DIR}/controller/tools/CircleCreationTool.h
    ${SRC_DIR}/controller/tools/CircleCreationTool.cpp
    ${SRC_DIR}/controller/tools/DeleteTool.h
    ${SRC_DIR}/controller/tools/DeleteTool.cpp
    ${SRC_DIR}/controller/tools/MoveTool.h
    ${SRC_DIR}/controller/tools/MoveTool.cpp
    ${SRC_DIR}/controller/tools/SegmentCreationTool.h
    ${SRC_DIR}/controller/tools/SegmentCreationTool.cpp
    ${SRC_DIR}/controller/tools/PolygonCreationTool.h
    ${SRC_DIR}/controller/tools/PolygonCreationTool.cpp
    ${SRC_DIR}/controller/tools/SplineCreationTool.h
    ${SRC_DIR}/controller/tools/SplineCreationTool.cpp
    ${SRC_DIR}/controller/tools/dimensions/LinearDimensionCreationTool.h
    ${SRC_DIR}/controller/tools/dimensions/LinearDimensionCreationTool.cpp

    #model
    ${SRC_DIR}/model/BasePrimitive.h
    ${SRC_DIR}/model/EnumManager.h
    ${SRC_DIR}/model/Scene.h
    ${SRC_DIR}/model/Scene.cpp

    #io
    ${SRC_DIR}/io/DxfExporter.h
    ${SRC_DIR}/io/DxfExporter.cpp
    ${SRC_DIR}/io/DxfImporter.h
    ${SRC_DIR}/io/DxfImporter.cpp

    ${SRC_DIR}/model/primitives/CirclePrimitive.h
    ${SRC_DIR}/model/primitives/CirclePrimitive.cpp
    ${SRC_DIR}/model/primitives/PointPrimitive.h
    ${SRC_DIR}/model/primitives/PointPrimitive.cpp
    ${SRC_DIR}/model/primitives/SegmentPrimitive.h
    ${SRC_DIR}/model/primitives/SegmentPrimitive.cpp
    ${SRC_DIR}/model/primitives/PolygonPrimitive.h
    ${SRC_DIR}/model/primitives/PolygonPrimitive.cpp
    ${SRC_DIR}/model/primitives/SplinePrimitive.h
    ${SRC_DIR}/model/primitives/SplinePrimitive.cpp
    ${SRC_DIR}/model/primitives/dimensions/BaseDimensionPrimitive.h
    ${SRC_DIR}/model/primitives/dimensions/BaseDimensionPrimitive.cpp
    ${SRC_DIR}/model/primitives/dimensions/LinearDimensionPrimitive.h
    ${SRC_DIR}/model/primitives/dimensions/LinearDimensionPrimitive.cpp

    #view
    ${SRC_DIR}/view/BaseDrawingTool.h
    ${SRC_DIR}/view/BasePanelWidget.h
    ${SRC_DIR}/view/BasePanelWidget.cpp
    ${SRC_DIR}/view/BasePropertiesWidget.h
    ${SRC_DIR}/view/BasePropertiesWidget.cpp
    ${SRC_DIR}/view/MainWindow.h
    ${SRC_DIR}/view/MainWindow.cpp
    ${SRC_DIR}/view/SettingsWindow.h
    ${SRC_DIR}/view/SettingsWindow.cpp
    ${SRC_DIR}/view/ViewportCamera.h
    ${SRC_DIR}/view/ViewportCamera.cpp

    ${SRC_DIR}/view/managers/AnimationManager.h
    ${SRC_DIR}/view/managers/AnimationManager.cpp
    ${SRC_DIR}/view/managers/LineStyleManager.h
    ${SRC_DIR}/view/managers/LineStyleManager.cpp
    ${SRC_DIR}/view/managers/SettingsManager.h
    ${SRC_DIR}/view/managers/SettingsManager.cpp
    ${SRC_DIR}/view/managers/ThemeManager.h
    ${SRC_DIR}/view/managers/ThemeManager.cpp
    ${SRC_DIR}/view/managers/SnapManager.h
    ${SRC_DIR}/view/managers/SnapManager.cpp

    ${SRC_DIR}/view/drawing/CircleDrawingTool.h
    ${SRC_DIR}/view/drawing/CircleDrawingTool.cpp
    ${SRC_DIR}/view/drawing/SegmentDrawingTool.h
    ${SRC_DIR}/view/drawing/SegmentDrawingTool.cpp
    ${SRC_DIR}/view/drawing/PolygonDrawingTool.h
    ${SRC_DIR}/view/drawing/PolygonDrawingTool.cpp
    ${SRC_DIR}/view/drawing/SplineDrawingTool.h
    ${SRC_DIR}/view/drawing/SplineDrawingTool.cpp

    ${SRC_DIR}/view/panels/ConsolePanelWidget.h
    ${SRC_DIR}/view/panels/ConsolePanelWidget.cpp
    ${SRC_DIR}/view/panels/NavigationPanelWidget.h
    ${SRC_DIR}/view/panels/NavigationPanelWidget.cpp
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

    ${SRC_DIR}/view/properties/CirclePropertiesWidget.h
    ${SRC_DIR}/view/properties/CirclePropertiesWidget.cpp
    ${SRC_DIR}/view/properties/SegmentPropertiesWidget.h
    ${SRC_DIR}/view/properties/SegmentPropertiesWidget.cpp
    ${SRC_DIR}/view/properties/CommonPropertiesWidget.h
    ${SRC_DIR}/view/properties/CommonPropertiesWidget.cpp
    ${SRC_DIR}/view/properties/PolygonPropertiesWidget.h
    ${SRC_DIR}/view/properties/PolygonPropertiesWidget.cpp
    ${SRC_DIR}/view/properties/SplinePropertiesWidget.h
    ${SRC_DIR}/view/properties/SplinePropertiesWidget.cpp
    ${SRC_DIR}/view/properties/DimensionPropertiesWidget.h
    ${SRC_DIR}/view/properties/DimensionPropertiesWidget.cpp

    ${SRC_DIR}/view/widgets/FlyoutToolButton.h
    ${SRC_DIR}/view/widgets/FlyoutToolButton.cpp

    #resources
    ${RES_DIR}/resources.qrc
)

#директории с заголовочными файлами
target_include_directories(ClarusCAD PRIVATE
    #controller
    ${SRC_DIR}/controller
    ${SRC_DIR}/controller/tools
    ${SRC_DIR}/controller/tools/dimensions

    #model
    ${SRC_DIR}/model
    ${SRC_DIR}/model/primitives
    ${SRC_DIR}/model/primitives/dimensions

    #io
    ${SRC_DIR}/io

    #view
    ${SRC_DIR}/view
    ${SRC_DIR}/view/drawing
    ${SRC_DIR}/view/managers
    ${SRC_DIR}/view/panels
    ${SRC_DIR}/view/properties
    ${SRC_DIR}/view/widgets
)

# ====================================================================
# НАСТРОЙКИ ДЛЯ MACOS
# ====================================================================
if(APPLE)
    #подключение необходимых библиотек
    target_link_libraries(ClarusCAD PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Svg
        "-framework AppKit"
        "-framework Foundation"
        "-framework IOKit"
        "-framework CoreGraphics"
    )

    #задание минимальной версии macOS (macOS 11 Big Sur)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum OS X deployment version" FORCE)

    #задание архитектуры процессора
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")

    #задание свойств конечного файла (Info.plist.in)
    set_target_properties(ClarusCAD PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packs/Info.plist.in"
    )

    #добавление лицензии
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/docs/LICENSE.txt")

    #поиск необходимых утилит для сборки проекта
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS ${Qt6_DIR}/../../../bin)
    find_program(PKGBUILD_EXECUTABLE pkgbuild)
    find_program(PRODUCTBUILD_EXECUTABLE productbuild)

    #копирование иконки в .app
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.icns")
        add_custom_command(TARGET ClarusCAD POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.icns" "$<TARGET_BUNDLE_DIR:ClarusCAD>/Contents/Resources/icon.icns"
        )
    endif()

    #создание и настройка .pkg
    if(PKGBUILD_EXECUTABLE AND PRODUCTBUILD_EXECUTABLE)
        add_custom_target(package
            #запуск macdeployqt
            COMMAND ${MACDEPLOYQT_EXECUTABLE} "$<TARGET_BUNDLE_DIR:ClarusCAD>" -verbose=1

            #подпись приложения
            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ClarusCAD>"

            #создание временной директории
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/package/root/Applications"

            #перемещение .app во временную директорию
            COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:ClarusCAD>" "${CMAKE_BINARY_DIR}/package/root/Applications/ClarusCAD.app"

            #упаковка в .pkg
            COMMAND ${PKGBUILD_EXECUTABLE}
                --root "${CMAKE_BINARY_DIR}/package/root"
                --identifier "com.clarus.cad"
                --version "${PROJECT_VERSION}"
                --install-location "/" "${CMAKE_BINARY_DIR}/package/ClarusCAD.pkg"

            #настройка .pkg
            COMMAND ${PRODUCTBUILD_EXECUTABLE}
                --package "${CMAKE_BINARY_DIR}/package/ClarusCAD.pkg" "${CMAKE_BINARY_DIR}/ClarusCAD_v${PROJECT_VERSION}_macOS_ARM.pkg"

            #удаление временной директории
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/package"

            DEPENDS ClarusCAD
            VERBATIM
        )
    endif()
endif()

# ====================================================================
# НАСТРОЙКИ ДЛЯ WINDOWS
# ====================================================================
if(WIN32)
    #подключение необходимых библиотек
    target_link_libraries(ClarusCAD PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Svg
    )

    #определение компилятором математических констант
    target_compile_definitions(ClarusCAD PRIVATE _USE_MATH_DEFINES)

    #задание свойств конечного файла
    set_target_properties(ClarusCAD PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    #добавление файла ресурсов (resources.rc)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packs/resources.rc")
        target_sources(ClarusCAD PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/packs/resources.rc)
    endif()

    #исполняемый файл помещается в /bin
    install(TARGETS ClarusCAD DESTINATION bin)

    #перемещение файла иконки
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.ico")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.ico" DESTINATION bin)
    endif()

    #настройка CPack для создания установщика
    set(CPACK_PACKAGE_NAME "ClarusCAD")
    set(CPACK_PACKAGE_VENDOR "Yashin Sergey")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "ClarusCAD_v${PROJECT_VERSION}_windows")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_INSTALL_PREFIX};.")

    #настройки для NSIS
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.ico")
    endif()
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/docs/LICENSE.txt")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\ClarusCAD.exe")
    set(CPACK_NSIS_DISPLAY_NAME "ClarusCAD")
    set(CPACK_NSIS_PACKAGE_NAME "ClarusCAD ${PROJECT_VERSION}")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\ClarusCAD.lnk' '$INSTDIR\\\\bin\\\\ClarusCAD.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\\\ClarusCAD.lnk'")

    #включение CPack
    include(CPack)
endif()

# ====================================================================
# НАСТРОЙКИ ДЛЯ LINUX
# ====================================================================
if(UNIX AND NOT APPLE)
    #подключение необходимых библиотек
    target_link_libraries(ClarusCAD PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Svg
    )

    #регистрация приложения (Info.desktop.in)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packs/Info.desktop.in")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/packs/Info.desktop.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Info.desktop"
            COPYONLY
        )
    endif()

    #исполняемый файл помещается в /bin
    install(TARGETS ClarusCAD DESTINATION bin)

    #копирование Info.desktop.in в /applications
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/Info.desktop")
        install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/Info.desktop"
            DESTINATION share/applications
            RENAME ClarusCAD.desktop
        )
    endif()

    #копирование иконки приложения
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.png")
        install(
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/app/icon.png"
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME ClarusCAD.png
        )
    endif()

    #добавление лицензии
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/LICENSE.txt"
        DESTINATION share/doc/claruscad
    )
endif()
