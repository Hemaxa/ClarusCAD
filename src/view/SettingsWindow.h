//SettingsWindow - класс окна настроек приложения

#pragma once

#include "EnumManager.h"

#include <QDialog>
#include <QVector>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTabWidget;
class QListWidget;
class QLineEdit;
class QLabel;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    //конструктор
    explicit SettingsWindow(QWidget* parent = nullptr);

private slots:
    //слот применения настроек
    void applySettings();

    //слоты для управления стилями линий
    void onAddStyleClicked();
    void onDeleteStyleClicked();

    //слоты конструктора паттерна
    void onAddDash();    // Добавить штрих
    void onAddDot();     // Добавить точку
    void onAddSpace();   // Добавить пробел
    void onClearPattern(); // Очистить паттерн

private:
    //вспомогательные методы создания вкладок
    QWidget* createAppearanceTab();
    QWidget* createViewportTab();
    QWidget* createLineStylesTab();

    //метод заполнения списка тем
    void populateThemeComboBox();

    //метод обновления текста предпросмотра паттерна
    void updatePatternPreview();

    //элементы интерфейса
    QTabWidget* m_tabWidget;

    //Элементы вкладки Оформление
    QComboBox* m_themeComboBox;

    //Элементы вкладки Рабочая область
    QSpinBox* m_gridStepSpinBox;
    QDoubleSpinBox* m_zoomStepSpinBox;
    QComboBox* m_angleUnitComboBox;

    //Элементы вкладки Стили линий
    QDoubleSpinBox* m_lineThicknessSpinBox;
    QDoubleSpinBox* m_dashLengthSpinBox; // <--- Новый
    QDoubleSpinBox* m_dashSpaceSpinBox;  // <--- Новый

    QListWidget* m_stylesListWidget;
    QLineEdit* m_styleNameEdit;

    //Вместо поля ввода теперь лейбл для отображения текущего паттерна
    QLabel* m_patternPreviewLabel;

    //Временное хранилище паттерна при создании
    QVector<qreal> m_currentPattern;
};
