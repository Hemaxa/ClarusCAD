//DxfExporter.h - класс для экспорта сцены в формат DXF

#pragma once

#include <QString>

class Scene;

class DxfExporter {
public:
    /**
     * @brief Экспорт сцены в файл формата DXF.
     * @param scene Ссылка на сцену для экспорта.
     * @param filePath Путь для сохранения файла.
     * @return true в случае успеха, false при ошибке.
     */
    static bool exportSceneToDxf(const Scene& scene, const QString& filePath);
};
