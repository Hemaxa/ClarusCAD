//SettingsWindow - класс окна настроек приложения

#pragma once

#include "EnumManager.h"

#include <QDialog>
#include <QVector>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QFontComboBox;
class QTabWidget;
class QListWidget;
class QLineEdit;
class QLabel;
class QCheckBox;
class QPushButton;

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
    void onEditStyleClicked();
    void onStyleSelectionChanged();

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
    QFontComboBox* m_dimensionFontComboBox;
    QDoubleSpinBox* m_dimensionTextHeightSpinBox;
    QDoubleSpinBox* m_dimensionTextGapSpinBox;
    QDoubleSpinBox* m_dimensionArrowSizeSpinBox;
    QComboBox* m_dimensionArrowTypeComboBox;
    QCheckBox* m_dimensionArrowFilledCheck;
    QDoubleSpinBox* m_dimensionExtOffsetSpinBox;
    QDoubleSpinBox* m_dimensionExtExtendSpinBox;
    QDoubleSpinBox* m_dimensionLineExtendSpinBox;
    QComboBox* m_dimensionExtLineTypeComboBox;
    QComboBox* m_dimensionLineTypeComboBox;
    QPushButton* m_dimensionTextColorButton;
    QPushButton* m_dimensionExtColorButton;
    QPushButton* m_dimensionLineColorButton;

    //Элементы вкладки Рабочая область
    QSpinBox* m_gridStepSpinBox;
    QDoubleSpinBox* m_zoomStepSpinBox;
    QComboBox* m_angleUnitComboBox;

    //Элементы вкладки Стили линий - Базовые параметры
    QDoubleSpinBox* m_lineThicknessSpinBox;
    QDoubleSpinBox* m_dashLengthSpinBox;
    QDoubleSpinBox* m_dashSpaceSpinBox;

    //Элементы вкладки Стили линий - Волнистая линия
    QDoubleSpinBox* m_waveAmplitudeSpinBox;
    QDoubleSpinBox* m_wavePeriodSpinBox;

    //Элементы вкладки Стили линий - Линия с изломами
    QDoubleSpinBox* m_kinkAmplitudeSpinBox;
    QDoubleSpinBox* m_kinkLengthSpinBox;
    QDoubleSpinBox* m_kinkStraightSpinBox;

    //Элементы конструктора пользовательских стилей
    QListWidget* m_stylesListWidget;
    QLineEdit* m_styleNameEdit;
    QLabel* m_patternPreviewLabel;
    QDoubleSpinBox* m_customThicknessSpinBox;
    QCheckBox* m_useCustomThicknessCheck;

    //хранилище паттерна для конструктора стилей
    QVector<qreal> m_currentPattern;
    int m_editingStyleId = -1; // ID редактируемого стиля (-1 = создание нового)
};
