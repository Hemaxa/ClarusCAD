//DxfImporter.h - класс для импорта сцены из формата DXF

#pragma once

#include <QString>

class Scene;

class DxfImporter {
public:
    /**
     * @brief Импорт сцены из файла формата DXF.
     * @param scene Ссылка на сцену для импорта.
     * @param filePath Путь к загружаемому файлу.
     * @return true в случае успеха, false при ошибке.
     */
    static bool importDxfToScene(Scene& scene, const QString& filePath);
};
