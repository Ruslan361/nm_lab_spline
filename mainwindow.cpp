#include "mainwindow.h"

// Удаляем менеджеры, их реализации методов теперь в MainWindow
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>
#include <QVariant>
#include <QHeaderView>
#include <limits> // Добавлен для findPlotBounds, createTableItem
#include <QPen> // Добавлен для drawCurve
#include <QBrush> // Добавлен для drawPoints
#include <QGraphicsTextItem> // Добавлен для drawAxisWithLabels
#include <QPainterPath> // Добавлен для drawCurve
#include <algorithm> // Добавлен для findPlotBounds (std::min/max)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    // , ui(new Ui::MainWindow) // Предполагаем, что ui не используется напрямую
{
    setupUI();
}

MainWindow::~MainWindow()
{
    delete solver;
    // Удаляем менеджеры, их удаление здесь не нужно
    // delete plotManager;
    // delete tableManager;
    // delete ui;
}

void MainWindow::setupUI() {
    createGraphicsViews();
    createTables();
    createControlPanel();
    createResultsPanel();
    layoutUI();

    // --- Удаляем создание менеджеров ---
    // plotManager = new PlotManager(...);
    // tableManager = new TableManager(...);

    // --- Подключаем сигналы ---
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveButtonClicked);
    connect(functionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFunctionChanged);
    connect(borderConditionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBorderConditionChanged);
    connect(plotTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tableTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    connect(showFunctionCheckbox, &QCheckBox::toggled, this, &MainWindow::onDisplayOptionsChanged);
    connect(showSplineCheckbox, &QCheckBox::toggled, this, &MainWindow::onDisplayOptionsChanged);
    connect(showControlPointsCheckbox, &QCheckBox::toggled, this, &MainWindow::onDisplayOptionsChanged);
    connect(showControlGridCheckbox, &QCheckBox::toggled, this, &MainWindow::onDisplayOptionsChanged);

    connect(zoomInButton, &QPushButton::clicked, this, &MainWindow::onZoomIn);
    connect(zoomOutButton, &QPushButton::clicked, this, &MainWindow::onZoomOut);
    connect(resetZoomButton, &QPushButton::clicked, this, &MainWindow::onResetZoom);
    connect(autoScaleCheckBox, &QCheckBox::toggled, this, &MainWindow::onAutoScaleChanged);

    connect(nodeCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::adjustControlGridNodes);
    connect(controlGridNodesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::validateControlGridNodes);
    connect(enforceGridMultipleCheckbox, &QCheckBox::toggled, this, &MainWindow::validateControlGridNodes);
    connect(useControlGridCheckbox, &QCheckBox::toggled, controlGridNodesSpinBox, &QSpinBox::setEnabled);
    connect(useControlGridCheckbox, &QCheckBox::toggled, enforceGridMultipleCheckbox, &QCheckBox::setEnabled);
    connect(borderConditionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBorderConditionTypeChanged);

    // --- Инициализация ---
    if (functionComboBox && functionComboBox->count() > 0) {
        onFunctionChanged(functionComboBox->currentIndex());
    }
    if (borderConditionComboBox && borderConditionComboBox->count() > 0) {
        onBorderConditionTypeChanged(borderConditionComboBox->currentIndex());
    }
    validateControlGridNodes();
}

// --- Методы создания UI (Оставляем без изменений) ---
void MainWindow::createGraphicsViews() {
    functionScene = new QGraphicsScene(this);
    functionView = new QGraphicsView(functionScene, this);
    derivativeScene = new QGraphicsScene(this);
    derivativeView = new QGraphicsView(derivativeScene, this);
    secondDerivativeScene = new QGraphicsScene(this);
    secondDerivativeView = new QGraphicsView(secondDerivativeScene, this);
    errorScene = new QGraphicsScene(this);
    errorView = new QGraphicsView(errorScene, this);
    errorDerivativeScene = new QGraphicsScene(this);
    errorDerivativeView = new QGraphicsView(errorDerivativeScene, this);
    errorSecondDerivativeScene = new QGraphicsScene(this);
    errorSecondDerivativeView = new QGraphicsView(errorSecondDerivativeScene, this);

    plotTabWidget = new QTabWidget(this);
    plotTabWidget->addTab(functionView, "Функция и сплайн");
    plotTabWidget->addTab(derivativeView, "Первая производная");
    plotTabWidget->addTab(secondDerivativeView, "Вторая производная");
    plotTabWidget->addTab(errorView, "Погрешность F");
    plotTabWidget->addTab(errorDerivativeView, "Погрешность F'");
    plotTabWidget->addTab(errorSecondDerivativeView, "Погрешность F''");
}

void MainWindow::createTables() {
    splineValuesTable = new QTableWidget(this);
    splineValuesTable->setColumnCount(8);
    splineValuesTable->setHorizontalHeaderLabels({"i", "x_i", "F(x_i)", "S(x_i)", "F-S", "F'(x_i)", "S'(x_i)", "F'-S'"});
    splineValuesTable->verticalHeader()->setVisible(false);
    splineValuesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    splineValuesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    derivativeValuesTable = new QTableWidget(this);
    derivativeValuesTable->setColumnCount(5);
    derivativeValuesTable->setHorizontalHeaderLabels({"i", "x_i", "F''(x_i)", "S''(x_i)", "F''-S''"});
    derivativeValuesTable->verticalHeader()->setVisible(false);
    derivativeValuesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    derivativeValuesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    coefficientTable = new QTableWidget(this);
    coefficientTable->setColumnCount(7);
    coefficientTable->setHorizontalHeaderLabels({"i", "x_i", "x_{i+1}", "a_i", "b_i", "c_i", "d_i"});
    coefficientTable->verticalHeader()->setVisible(false);
    coefficientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    coefficientTable->setSelectionBehavior(QAbstractItemView::SelectRows);


    tableTabWidget = new QTabWidget(this);
    tableTabWidget->addTab(splineValuesTable, "Значения F, S, F', S'");
    tableTabWidget->addTab(derivativeValuesTable, "Значения F'', S''");
    tableTabWidget->addTab(coefficientTable, "Коэффициенты сплайна");
}

void MainWindow::createControlPanel() {
    QGroupBox* gridGroupBox = new QGroupBox("Настройки сетки и интервала", this);
    QVBoxLayout* gridLayout = new QVBoxLayout(gridGroupBox);

    // Интервал [a, b]
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("Интервал [a, b]:"));
    intervalStartSpinBox = new QDoubleSpinBox(this); // Создание объекта
    intervalStartSpinBox->setRange(-1000.0, 1000.0);
    intervalStartSpinBox->setValue(-1.0);
    intervalStartSpinBox->setDecimals(4);
    intervalLayout->addWidget(intervalStartSpinBox);
    intervalLayout->addWidget(new QLabel(" до "));
    intervalEndSpinBox = new QDoubleSpinBox(this); // Создание объекта
    intervalEndSpinBox->setRange(-1000.0, 1000.0);
    intervalEndSpinBox->setValue(1.0);
    intervalEndSpinBox->setDecimals(4);
    intervalLayout->addWidget(intervalEndSpinBox);
    gridLayout->addLayout(intervalLayout);


    // Основная сетка
    QHBoxLayout* splineGridLayout = new QHBoxLayout();
    splineGridLayout->addWidget(new QLabel("Узлы сплайна (n):"));
    nodeCountSpinBox = new QSpinBox(this);
    nodeCountSpinBox->setMinimum(4);
    nodeCountSpinBox->setMaximum(1000);
    nodeCountSpinBox->setValue(10);
    splineGridLayout->addWidget(nodeCountSpinBox);
    gridLayout->addLayout(splineGridLayout);

    // Контрольная сетка
    QHBoxLayout* controlGridLayout = new QHBoxLayout();
    useControlGridCheckbox = new QCheckBox("Использовать контрольную сетку", this);
    useControlGridCheckbox->setChecked(true);
    controlGridLayout->addWidget(useControlGridCheckbox);
    controlGridLayout->addWidget(new QLabel("Узлы (N):"));
    controlGridNodesSpinBox = new QSpinBox(this);
    controlGridNodesSpinBox->setMinimum(10);
    controlGridNodesSpinBox->setMaximum(5000);
    controlGridNodesSpinBox->setValue(500); // Увеличиваем количество точек по умолчанию для более гладкого графика
    controlGridNodesSpinBox->setSingleStep(50); // Увеличиваем шаг изменения для удобства
    controlGridLayout->addWidget(controlGridNodesSpinBox);
    gridLayout->addLayout(controlGridLayout);

    // Кратность контрольной сетки
    QHBoxLayout* gridMultipleLayout = new QHBoxLayout();
    enforceGridMultipleCheckbox = new QCheckBox("Контрольная сетка кратна основной", this);
    enforceGridMultipleCheckbox->setChecked(true);
    gridMultipleLayout->addWidget(enforceGridMultipleCheckbox);
    gridLayout->addLayout(gridMultipleLayout);

    // Подключение сигналов для проверки кратности сеток
    connect(nodeCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::adjustControlGridNodes);
    connect(controlGridNodesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::validateControlGridNodes);
    connect(enforceGridMultipleCheckbox, &QCheckBox::toggled, this, &MainWindow::validateControlGridNodes);

    connect(useControlGridCheckbox, &QCheckBox::toggled, controlGridNodesSpinBox, &QSpinBox::setEnabled);
    connect(useControlGridCheckbox, &QCheckBox::toggled, enforceGridMultipleCheckbox, &QCheckBox::setEnabled); // Исправленная цель

    // Создаем группу с функциями
    QGroupBox* functionGroupBox = new QGroupBox("Выбор функции", this);
    QVBoxLayout* functionLayout = new QVBoxLayout(functionGroupBox);

    functionComboBox = new QComboBox(this);
    functionComboBox->addItem("Тестовая функция: x^2", QVariant::fromValue(TEST));
    functionComboBox->addItem("sqrt(|x^2-1|)/x", QVariant::fromValue(MAIN1));
    functionComboBox->addItem("(1+x^2)^(1/3)", QVariant::fromValue(MAIN1));
    functionComboBox->addItem("sin(x+1)/(x+1)", QVariant::fromValue(MAIN1));
    functionComboBox->addItem("log(1+x)/(1+x)", QVariant::fromValue(MAIN1));
    functionComboBox->addItem("Осциллирующая: sin(1/x)", QVariant::fromValue(OSC));
    functionLayout->addWidget(functionComboBox);

    // Создаем группу с граничными условиями
    QGroupBox* boundaryGroupBox = new QGroupBox("Граничные условия", this);
    QVBoxLayout* boundaryLayout = new QVBoxLayout(boundaryGroupBox);

    // Тип сетки
    gridTypeComboBox = new QComboBox(this);
    gridTypeComboBox->addItem("Равномерная сетка", QVariant::fromValue((int)UNIFORM_GRID)); // Cast to int
    // gridTypeComboBox->addItem("Неравномерная сетка", QVariant::fromValue(NON_UNIFORM_GRID));
    boundaryLayout->addWidget(gridTypeComboBox);

    // Тип граничных условий
    borderConditionComboBox = new QComboBox(this);
    borderConditionComboBox->addItem("Естественный сплайн (S''=0)", QVariant::fromValue(NATURAL));
    borderConditionComboBox->addItem("Заданные первые производные S'(a), S'(b)", QVariant::fromValue(DERIVATIVE_BOUNDS));
    borderConditionComboBox->addItem("Заданные вторые производные S''(a), S''(b)", QVariant::fromValue(SECOND_DERIVATIVE_BOUNDS));
    boundaryLayout->addWidget(borderConditionComboBox);

    // Поля для ввода значений граничных условий
    QHBoxLayout* boundaryValueLayout = new QHBoxLayout();
    boundaryStartLabel = new QLabel("S'(a):"); // Создание объекта
    boundaryValueLayout->addWidget(boundaryStartLabel);
    boundaryStartSpinBox = new QDoubleSpinBox(this); // Создание объекта
    boundaryStartSpinBox->setRange(-1000.0, 1000.0);
    boundaryStartSpinBox->setValue(0.0);
    boundaryStartSpinBox->setDecimals(4);
    boundaryValueLayout->addWidget(boundaryStartSpinBox);

    boundaryEndLabel = new QLabel("S'(b):"); // Создание объекта
    boundaryValueLayout->addWidget(boundaryEndLabel);
    boundaryEndSpinBox = new QDoubleSpinBox(this); // Создание объекта
    boundaryEndSpinBox->setRange(-1000.0, 1000.0);
    boundaryEndSpinBox->setValue(0.0);
    boundaryEndSpinBox->setDecimals(4);
    boundaryValueLayout->addWidget(boundaryEndSpinBox);
    boundaryLayout->addLayout(boundaryValueLayout);

    // Изначально скрываем поля для гр. условий
    boundaryStartLabel->setVisible(false);
    boundaryStartSpinBox->setVisible(false);
    boundaryEndLabel->setVisible(false);
    boundaryEndSpinBox->setVisible(false);

    // Подключаем сигнал для изменения видимости полей гр. условий
    connect(borderConditionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBorderConditionTypeChanged);


    // Создаем группу настроек отображения
    QGroupBox* displayGroupBox = new QGroupBox("Отображение графиков", this);
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroupBox);

    showFunctionCheckbox = new QCheckBox("Показывать исходную функцию", this);
    showFunctionCheckbox->setChecked(true);
    displayLayout->addWidget(showFunctionCheckbox);

    showSplineCheckbox = new QCheckBox("Показывать сплайн", this);
    showSplineCheckbox->setChecked(true);
    displayLayout->addWidget(showSplineCheckbox);

    showControlPointsCheckbox = new QCheckBox("Показывать узлы сплайна", this);
    showControlPointsCheckbox->setChecked(true);
    displayLayout->addWidget(showControlPointsCheckbox);

    showControlGridCheckbox = new QCheckBox("Показывать точки контрольной сетки", this);
    showControlGridCheckbox->setChecked(true);
    displayLayout->addWidget(showControlGridCheckbox);

    // Создаем группу масштабирования
    QGroupBox* zoomGroupBox = new QGroupBox("Масштабирование", this);
    QGridLayout* zoomLayout = new QGridLayout(zoomGroupBox);

    zoomInButton = new QPushButton("+", this);
    zoomOutButton = new QPushButton("-", this);
    resetZoomButton = new QPushButton("Сброс", this);
    autoScaleCheckBox = new QCheckBox("Автомасштаб", this);
    autoScaleCheckBox->setChecked(true);

    zoomLayout->addWidget(zoomInButton, 0, 0);
    zoomLayout->addWidget(zoomOutButton, 0, 1);
    zoomLayout->addWidget(resetZoomButton, 0, 2);
    zoomLayout->addWidget(autoScaleCheckBox, 1, 0, 1, 3);

    // Кнопка расчета
    solveButton = new QPushButton("Построить сплайн", this);
    
    // Кнопка вызова справки
    QPushButton* helpButton = new QPushButton("Справка", this);
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::showHelpDialog);

    // Создаем контейнер для групп опций
    QVBoxLayout* controlLayout = new QVBoxLayout();
    controlLayout->addWidget(gridGroupBox);
    controlLayout->addWidget(functionGroupBox);
    controlLayout->addWidget(boundaryGroupBox);
    controlLayout->addWidget(displayGroupBox);
    controlLayout->addWidget(zoomGroupBox);
    controlLayout->addWidget(solveButton);
    controlLayout->addWidget(helpButton);
    controlLayout->addStretch();

    // Создаем виджет для размещения контрольных элементов
    controlPanelWidget = new QWidget(this);
    controlPanelWidget->setLayout(controlLayout);
}

void MainWindow::createResultsPanel() {
    resultsPanel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(resultsPanel);
    
    // Создаем группу для информации о сетке с компактным вертикальным дизайном
    QGroupBox* gridInfoBox = new QGroupBox("Информация о сетке", this);
    gridInfoBox->setMaximumHeight(150); // Увеличиваем высоту для вертикального размещения
    gridInfoBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QVBoxLayout* gridInfoLayout = new QVBoxLayout(gridInfoBox); // Вертикальный макет
    gridInfoLayout->setSpacing(2); // Уменьшаем отступы
    
    // Компактные метки с более краткими названиями
    QLabel* lblSplineGrid = new QLabel("Сетка:", this);
    splineGridInfoLabel = new QLabel("-", this);
    splineGridInfoLabel->setWordWrap(true); // Разрешаем перенос текста
    
    QLabel* lblGridStep = new QLabel("Шаг сетки:", this);
    gridStepInfoLabel = new QLabel("-", this);
    
    QLabel* lblControlGrid = new QLabel("Контр. сетка:", this);
    controlGridInfoLabel = new QLabel("-", this);
    controlGridInfoLabel->setWordWrap(true); // Разрешаем перенос текста
    
    // Добавляем элементы вертикально
    gridInfoLayout->addWidget(lblSplineGrid);
    gridInfoLayout->addWidget(splineGridInfoLabel);
    gridInfoLayout->addWidget(lblGridStep);
    gridInfoLayout->addWidget(gridStepInfoLabel);
    gridInfoLayout->addWidget(lblControlGrid);
    gridInfoLayout->addWidget(controlGridInfoLabel);
    
    // Создаем группу для информации о функции
    QGroupBox* funcInfoBox = new QGroupBox("Функция", this);
    funcInfoBox->setMaximumHeight(80);
    funcInfoBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QVBoxLayout* funcInfoLayout = new QVBoxLayout(funcInfoBox);
    funcInfoLayout->setSpacing(2);
    
    functionInfoLabel = new QLabel("-", this);
    functionInfoLabel->setWordWrap(true); // Разрешаем перенос текста
    funcInfoLayout->addWidget(functionInfoLabel);
    
    // Создаем группу для погрешностей
    QGroupBox* errorInfoBox = new QGroupBox("Погрешности", this);
    //errorInfoBox->setMaximumHeight(150); // Увеличиваем высоту для вертикального размещения
    //errorInfoBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QVBoxLayout* errorInfoLayout = new QVBoxLayout(errorInfoBox); // Вертикальный макет
    errorInfoLayout->setSpacing(2);
    
    QLabel* lblErrorF = new QLabel("F:", this);
    errorFunctionLabel = new QLabel("-", this);
    errorFunctionLabel->setWordWrap(true); // Разрешаем перенос текста
    
    QLabel* lblErrorDF = new QLabel("F':", this);
    errorDerivativeLabel = new QLabel("-", this);
    errorDerivativeLabel->setWordWrap(true); // Разрешаем перенос текста
    
    QLabel* lblErrorD2F = new QLabel("F'':", this);
    errorSecondDerivativeLabel = new QLabel("-", this);
    errorSecondDerivativeLabel->setWordWrap(true); // Разрешаем перенос текста
    
    // Добавляем элементы вертикально
    errorInfoLayout->addWidget(lblErrorF);
    errorInfoLayout->addWidget(errorFunctionLabel);
    errorInfoLayout->addWidget(lblErrorDF);
    errorInfoLayout->addWidget(errorDerivativeLabel);
    errorInfoLayout->addWidget(lblErrorD2F);
    errorInfoLayout->addWidget(errorSecondDerivativeLabel);
    
    // Добавляем группы в основной макет
    layout->addWidget(gridInfoBox);
    layout->addWidget(funcInfoBox);
    layout->addWidget(errorInfoBox);
    layout->addStretch();
}

void MainWindow::layoutUI() {
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainHLayout = new QHBoxLayout(centralWidget);
    mainHLayout->setContentsMargins(10, 10, 10, 10);
    mainHLayout->setSpacing(10);

    mainHLayout->addWidget(controlPanelWidget); // Левая панель

    QVBoxLayout *rightPanelLayout = new QVBoxLayout(); // Правая панель
    rightPanelLayout->addWidget(plotTabWidget);    // Вверху: Графики
    rightPanelLayout->addWidget(tableTabWidget);   // Внизу: Таблицы

    mainHLayout->addLayout(rightPanelLayout);      // Средняя панель
    mainHLayout->addWidget(resultsPanel);          // Самая правая панель

    // Изменение соотношения ширин панелей для более узкой правой панели
    mainHLayout->setStretch(0, 1);    // Ширина панели управления
    mainHLayout->setStretch(1, 4);    // Увеличиваем относительную ширину графиков/таблиц
    mainHLayout->setStretch(2, 0.7);  // Уменьшаем относительную ширину правой панели с результатами

    setCentralWidget(centralWidget);
    setWindowTitle("Интерполяция кубическим сплайном");
    resize(1400, 900);
}

// --- Реализации слотов (Обновляем вызовы методов менеджеров на вызовы методов MainWindow) ---
void MainWindow::onSolveButtonClicked()
{
    int nodeCount = nodeCountSpinBox->value();
    MODE functionMode = static_cast<MODE>(functionComboBox->currentData().toInt());
    BORDER_MODE borderMode = static_cast<BORDER_MODE>(borderConditionComboBox->currentData().toInt());
    double intervalStart = intervalStartSpinBox ? intervalStartSpinBox->value() : -1.0;
    double intervalEnd = intervalEndSpinBox ? intervalEndSpinBox->value() : 1.0;
    double boundaryStartVal = boundaryStartSpinBox ? boundaryStartSpinBox->value() : 0.0;
    double boundaryEndVal = boundaryEndSpinBox ? boundaryEndSpinBox->value() : 0.0;

    try {
        if (intervalEnd <= intervalStart) {
             QMessageBox::warning(this, "Ошибка ввода", "Конец интервала 'b' должен быть больше начала 'a'.");
             return;
        }

        delete solver;
        solver = new Solver(nodeCount, functionMode, borderMode, intervalStart, intervalEnd, boundaryStartVal, boundaryEndVal);
        solver->Solve();

        // --- Прямые вызовы методов MainWindow ---
        updatePlots();
        fillTables();
        updateInfoLabels();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", QString("Произошла ошибка при расчете: %1").arg(e.what()));
        delete solver;
        solver = nullptr;
        // Очистка графиков и таблиц в случае ошибки
        updatePlots(); // Очистит сцены, если solver равен нулю
        fillTables(); // Очистит таблицы, если solver равен нулю
        updateInfoLabels();
    }
}

void MainWindow::onFunctionChanged(int index) {
    // Прямые вызовы методов менеджеров здесь не нужны
}

void MainWindow::onBorderConditionChanged(int index) {
    // Прямые вызовы методов менеджеров здесь не нужны
}

void MainWindow::onTabChanged(int index) {
    // Прямые вызовы методов менеджеров здесь не нужны
    // updatePlots(); // Необязательно: Перерисовать при смене вкладки, если нужно
}

void MainWindow::onDisplayOptionsChanged() {
    updatePlots(); // Вызываем метод MainWindow
}

void MainWindow::onZoomIn() {
    zoom(1.2); // Вызываем метод MainWindow
}

void MainWindow::onZoomOut() {
    zoom(1.0 / 1.2); // Вызываем метод MainWindow
}

void MainWindow::onResetZoom() {
    resetView(); // Вызываем метод MainWindow
}

void MainWindow::onAutoScaleChanged(bool checked) {
    updatePlots(); // Вызываем метод MainWindow
}

void MainWindow::adjustControlGridNodes(int n_value) {
    validateControlGridNodes();
    // if (useControlGridCheckbox->isChecked()) {
    //     fillTables(); // Обновить таблицы, если изменяется контрольная сетка
    // }
}

void MainWindow::validateControlGridNodes() {
    if (!enforceGridMultipleCheckbox || !nodeCountSpinBox || !controlGridNodesSpinBox) return;

    if (enforceGridMultipleCheckbox->isChecked()) {
        int n = nodeCountSpinBox->value();
        int N = controlGridNodesSpinBox->value();
        if (n <= 1) return;

        int n_intervals = n - 1;
        int N_intervals = N - 1;

        if (N_intervals % n_intervals != 0) {
            int new_N_intervals = ((N_intervals / n_intervals) + 1) * n_intervals;
            int new_N = new_N_intervals + 1;
            controlGridNodesSpinBox->blockSignals(true);
            controlGridNodesSpinBox->setValue(new_N);
            controlGridNodesSpinBox->blockSignals(false);
            // Опционально обновить таблицы, если используется контрольная сетка
            // if (useControlGridCheckbox->isChecked()) {
            //     fillTables();
            // }
        }
    }
}

void MainWindow::onBorderConditionTypeChanged(int index) {
    BORDER_MODE selectedMode = static_cast<BORDER_MODE>(borderConditionComboBox->itemData(index).toInt());
    bool showFields = (selectedMode == DERIVATIVE_BOUNDS || selectedMode == SECOND_DERIVATIVE_BOUNDS);
    
    // Определяем текст меток в зависимости от выбранного типа граничных условий
    QString startLabel, endLabel;
    if (selectedMode == DERIVATIVE_BOUNDS) {
        startLabel = "S'(a):";
        endLabel = "S'(b):";
    } else if (selectedMode == SECOND_DERIVATIVE_BOUNDS) {
        startLabel = "S''(a):";
        endLabel = "S''(b):";
    }

    if (boundaryStartLabel) boundaryStartLabel->setVisible(showFields);
    if (boundaryStartSpinBox) boundaryStartSpinBox->setVisible(showFields);
    if (boundaryEndLabel) boundaryEndLabel->setVisible(showFields);
    if (boundaryEndSpinBox) boundaryEndSpinBox->setVisible(showFields);

    if (showFields) {
        if (boundaryStartLabel) boundaryStartLabel->setText(startLabel);
        if (boundaryEndLabel) boundaryEndLabel->setText(endLabel);
    }
}

// --- Объединенные реализации методов PlotManager ---

void MainWindow::updatePlots() {
    if (!solver) {
        // Очистить все сцены, если нет данных от solver
        if(functionScene) functionScene->clear();
        if(derivativeScene) derivativeScene->clear();
        if(secondDerivativeScene) secondDerivativeScene->clear();
        if(errorScene) errorScene->clear();
        if(errorDerivativeScene) errorDerivativeScene->clear();
        if(errorSecondDerivativeScene) errorSecondDerivativeScene->clear();
        return;
    }

    // Получаем количество узлов контрольной сетки
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    // Вычисляем погрешности на контрольной сетке перед отрисовкой графиков
    solver->calculateErrorsOnControlGrid(N);

    drawFunctionPlot();
    drawDerivativePlot();
    drawSecondDerivativePlot();
    drawErrorPlot();
    drawDerivativeErrorPlot();
    drawSecondDerivativeErrorPlot();
    
    // Обновляем информацию о погрешностях после построения графиков
    updateInfoLabels();
}

void MainWindow::zoom(double factor) {
    if (functionView) functionView->scale(factor, factor);
    if (derivativeView) derivativeView->scale(factor, factor);
    if (secondDerivativeView) secondDerivativeView->scale(factor, factor);
    if (errorView) errorView->scale(factor, factor);
    if (errorDerivativeView) errorDerivativeView->scale(factor, factor);
    if (errorSecondDerivativeView) errorSecondDerivativeView->scale(factor, factor);
}

void MainWindow::resetView() {
    if (functionView) functionView->resetTransform();
    if (derivativeView) derivativeView->resetTransform();
    if (secondDerivativeView) secondDerivativeView->resetTransform();
    if (errorView) errorView->resetTransform();
    if (errorDerivativeView) errorDerivativeView->resetTransform();
    if (errorSecondDerivativeView) errorSecondDerivativeView->resetTransform();
    updatePlots(); // Перерисовать после сброса
}

void MainWindow::findPlotBounds(const std::vector<double>& x,
                                const std::vector<double>& y1, const std::vector<double>& y2,
                                double& minX, double& maxX, double& minY, double& maxY)
{
    if (x.empty()) return;

    minX = x.front();
    maxX = x.back();
    minY = std::numeric_limits<double>::max();
    maxY = std::numeric_limits<double>::lowest();

    auto updateMinMax = [&](const std::vector<double>& y) {
        for (double val : y) {
            if (!std::isnan(val) && !std::isinf(val)) {
                minY = std::min(minY, val);
                maxY = std::max(maxY, val);
            }
        }
    };

    updateMinMax(y1);
    if (!y2.empty()) {
        updateMinMax(y2);
    }

    if (minY > maxY) {
        minY = -1.0;
        maxY = 1.0;
    } else if (minY == maxY) {
        minY -= 0.5;
        maxY += 0.5;
    }

    double yPadding = (maxY - minY) * 0.1;
    minY -= yPadding;
    maxY += yPadding;
    if (minY == maxY) {
        minY -= 0.5;
        maxY += 0.5;
    }
}

void MainWindow::drawFunctionPlot()
{
    if (!solver || !functionScene || !functionView || !autoScaleCheckBox ||
        !showFunctionCheckbox || !showSplineCheckbox || !showControlPointsCheckbox) return;

    functionScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков
    std::vector<double> x, f, s;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    const auto& f_nodes = solver->getF_nodes(); // Значения функции в узлах
    
    if (x_nodes.empty() || f_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    f.resize(N);
    s.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        f[i] = solver->getProblemFunc(x[i]);
        s[i] = solver->getSpline()(x[i]);
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, f, s, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = -2; maxY = 2; // Фиксированные границы
    }

    drawAxisWithLabels(functionScene, functionView, minX, maxX, minY, maxY, "x", "F(x), S(x)");

    if (showFunctionCheckbox->isChecked()) {
        drawCurve(functionScene, functionView, x, f, minX, maxX, minY, maxY, QPen(Qt::blue, 2));
    }
    if (showSplineCheckbox->isChecked()) {
        drawCurve(functionScene, functionView, x, s, minX, maxX, minY, maxY, QPen(Qt::red, 2, Qt::DashLine));
    }
    if (showControlPointsCheckbox->isChecked()) {
        // Рисуем узлы сплайна
        drawPoints(functionScene, functionView, x_nodes, f_nodes, minX, maxX, minY, maxY, QBrush(Qt::black));
    }
    // Отображаем точки контрольной сетки, если нужно
    if (showControlGridCheckbox && showControlGridCheckbox->isChecked()) {
        // Рисуем только небольшую выборку точек для наглядности
        std::vector<double> x_sample, s_sample;
        int step = N / 50; // Отображаем примерно 50 точек
        if (step < 1) step = 1;
        
        for (size_t i = 0; i < x.size(); i += step) {
            x_sample.push_back(x[i]);
            s_sample.push_back(s[i]);
        }
        drawPoints(functionScene, functionView, x_sample, s_sample, minX, maxX, minY, maxY, QBrush(Qt::green), 2.0);
    }
}

void MainWindow::drawDerivativePlot()
{
    if (!solver || !derivativeScene || !derivativeView || !autoScaleCheckBox ||
        !showFunctionCheckbox || !showSplineCheckbox) return;

    derivativeScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков производных
    std::vector<double> x, df, ds;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    
    if (x_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    df.resize(N);
    ds.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        df[i] = solver->getProblemDeriv(x[i]);
        ds[i] = solver->getSpline().ds(x[i]);
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, df, ds, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = -5; maxY = 5;
    }

    drawAxisWithLabels(derivativeScene, derivativeView, minX, maxX, minY, maxY, "x", "F'(x), S'(x)");

    if (showFunctionCheckbox->isChecked()) {
        drawCurve(derivativeScene, derivativeView, x, df, minX, maxX, minY, maxY, QPen(Qt::blue, 2));
    }
    if (showSplineCheckbox->isChecked()) {
        drawCurve(derivativeScene, derivativeView, x, ds, minX, maxX, minY, maxY, QPen(Qt::red, 2, Qt::DashLine));
    }
}

void MainWindow::drawSecondDerivativePlot()
{
    if (!solver || !secondDerivativeScene || !secondDerivativeView || !autoScaleCheckBox ||
        !showFunctionCheckbox || !showSplineCheckbox) return;

    secondDerivativeScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков производных
    std::vector<double> x, d2f, d2s;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    
    if (x_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    d2f.resize(N);
    d2s.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        d2f[i] = solver->getProblemDeriv2(x[i]);
        d2s[i] = solver->getSpline().d2s(x[i]);
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, d2f, d2s, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = -10; maxY = 10;
    }

    drawAxisWithLabels(secondDerivativeScene, secondDerivativeView, minX, maxX, minY, maxY, "x", "F''(x), S''(x)");

    if (showFunctionCheckbox->isChecked()) {
        drawCurve(secondDerivativeScene, secondDerivativeView, x, d2f, minX, maxX, minY, maxY, QPen(Qt::blue, 2));
    }
    if (showSplineCheckbox->isChecked()) {
        drawCurve(secondDerivativeScene, secondDerivativeView, x, d2s, minX, maxX, minY, maxY, QPen(Qt::red, 2, Qt::DashLine));
    }
}

void MainWindow::drawErrorPlot()
{
    if (!solver || !errorScene || !errorView || !autoScaleCheckBox) return;

    errorScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков
    std::vector<double> x, f, s, error;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    
    if (x_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    error.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        double f_val = solver->getProblemFunc(x[i]);
        double s_val = solver->getSpline()(x[i]);
        error[i] = (!std::isnan(f_val) && !std::isnan(s_val)) ? std::abs(f_val - s_val) : std::numeric_limits<double>::quiet_NaN();
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = 0; maxY = 1;
    }

    drawAxisWithLabels(errorScene, errorView, minX, maxX, minY, maxY, "x", "|F(x) - S(x)|");
    drawCurve(errorScene, errorView, x, error, minX, maxX, minY, maxY, QPen(Qt::magenta, 2));
}

void MainWindow::drawDerivativeErrorPlot()
{
    if (!solver || !errorDerivativeScene || !errorDerivativeView || !autoScaleCheckBox) return;

    errorDerivativeScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков ошибок производных
    std::vector<double> x, error;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    
    if (x_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    error.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        double df_val = solver->getProblemDeriv(x[i]);
        double ds_val = solver->getSpline().ds(x[i]);
        error[i] = (!std::isnan(df_val) && !std::isnan(ds_val)) ? std::abs(df_val - ds_val) : std::numeric_limits<double>::quiet_NaN();
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = 0; maxY = 2;
    }

    drawAxisWithLabels(errorDerivativeScene, errorDerivativeView, minX, maxX, minY, maxY, "x", "|F'(x) - S'(x)|");
    drawCurve(errorDerivativeScene, errorDerivativeView, x, error, minX, maxX, minY, maxY, QPen(Qt::magenta, 2));
}

void MainWindow::drawSecondDerivativeErrorPlot()
{
    if (!solver || !errorSecondDerivativeScene || !errorSecondDerivativeView || !autoScaleCheckBox) return;

    errorSecondDerivativeScene->clear();
    
    // Создаем более плотную сетку точек для гладкого отображения графиков ошибок вторых производных
    std::vector<double> x, error;
    
    const auto& x_nodes = solver->getX_nodes(); // Узлы сплайна
    
    if (x_nodes.empty()) return;
    
    double a = x_nodes.front();
    double b = x_nodes.back();
    
    // Используем больше точек для гладкого отображения
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : 500;
    if (N < 500) N = 500; // Минимум 500 точек для графиков
    
    x.resize(N);
    error.resize(N);
    
    // Генерируем плотную сетку с гарантированными точными значениями на границах
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            x[i] = a; // Точно a для первой точки
        } else if (i == N - 1) {
            x[i] = b; // Точно b для последней точки
        } else {
            x[i] = a + i * (b - a) / (N - 1);
        }
        
        double d2f_val = solver->getProblemDeriv2(x[i]);
        double d2s_val = solver->getSpline().d2s(x[i]);
        error[i] = (!std::isnan(d2f_val) && !std::isnan(d2s_val)) ? std::abs(d2f_val - d2s_val) : std::numeric_limits<double>::quiet_NaN();
    }

    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    if (autoScaleCheckBox->isChecked()) {
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY);
    } else {
        minX = a; maxX = b; minY = 0; maxY = 5;
    }

    drawAxisWithLabels(errorSecondDerivativeScene, errorSecondDerivativeView, minX, maxX, minY, maxY, "x", "|F''(x) - S''(x)|");
    drawCurve(errorSecondDerivativeScene, errorSecondDerivativeView, x, error, minX, maxX, minY, maxY, QPen(Qt::magenta, 2));
}

void MainWindow::drawAxisWithLabels(QGraphicsScene* scene, QGraphicsView* view,
                                    double minX, double maxX, double minY, double maxY,
                                    const QString& xLabel, const QString& yLabel)
{
    if (!scene || !view) return;

    // Получаем размеры вида
    QRectF viewRect = view->rect();
    qreal viewWidth = viewRect.width() - 20; // Отступ
    qreal viewHeight = viewRect.height() - 20;

    // Вычисляем коэффициенты масштабирования
    double scaleX = (maxX - minX == 0) ? 1 : viewWidth / (maxX - minX);
    double scaleY = (maxY - minY == 0) ? 1 : viewHeight / (maxY - minY);

    // --- Рисуем оси ---
    QPen axisPen(Qt::black, 1);

    // Ось X (горизонтальная)
    double yOriginView = viewRect.bottom() - 10 - ((0 - minY) * scaleY);
    if (yOriginView < 10) yOriginView = 10; // Ограничиваем сверху
    if (yOriginView > viewRect.bottom() - 10) yOriginView = viewRect.bottom() - 10; // Ограничиваем снизу
    scene->addLine(10, yOriginView, viewRect.right() - 10, yOriginView, axisPen);

    // Ось Y (вертикальная)
    double xOriginView = 10 + ((0 - minX) * scaleX);
    if (xOriginView < 10) xOriginView = 10; // Ограничиваем слева
    if (xOriginView > viewRect.right() - 10) xOriginView = viewRect.right() - 10; // Ограничиваем справа
    scene->addLine(xOriginView, 10, xOriginView, viewRect.bottom() - 10, axisPen);

    // --- Рисуем деления и подписи ---
    int numTicks = 5; // Количество делений на каждой оси
    QFont labelFont("Arial", 8);

    // Деления и подписи оси X
    for (int i = 0; i <= numTicks; ++i) {
        double xVal = minX + (maxX - minX) * i / numTicks;
        double xView = 10 + (xVal - minX) * scaleX;
        scene->addLine(xView, yOriginView - 3, xView, yOriginView + 3, axisPen); // Деление
        QGraphicsTextItem* label = scene->addText(QString::number(xVal, 'g', 3), labelFont);
        label->setPos(xView - label->boundingRect().width() / 2, yOriginView + 5);
    }

    // Деления и подписи оси Y
    for (int i = 0; i <= numTicks; ++i) {
        double yVal = minY + (maxY - minY) * i / numTicks;
        double yView = viewRect.bottom() - 10 - (yVal - minY) * scaleY;
        scene->addLine(xOriginView - 3, yView, xOriginView + 3, yView, axisPen); // Деление
        QGraphicsTextItem* label = scene->addText(QString::number(yVal, 'g', 3), labelFont);
        label->setPos(xOriginView - label->boundingRect().width() - 5, yView - label->boundingRect().height() / 2);
    }

    // Подписи осей
    QGraphicsTextItem* xLabelItem = scene->addText(xLabel, QFont("Arial", 10, QFont::Bold));
    xLabelItem->setPos(viewRect.right() - 40 - xLabelItem->boundingRect().width(), 
                      viewRect.bottom() - 30);

    QGraphicsTextItem* yLabelItem = scene->addText(yLabel, QFont("Arial", 10, QFont::Bold));
    yLabelItem->setPos(20, 10);
    
    // Добавляем легенду
    if (yLabel.contains(",")) {
        // Легенда нужна только для графиков с несколькими линиями
        QGraphicsRectItem* legendBox = scene->addRect(viewRect.right() - 150, 10, 140, 50, 
                                                    QPen(Qt::black), QBrush(QColor(255, 255, 255, 200)));
        
        // Линия и текст для функции
        scene->addLine(viewRect.right() - 140, 20, viewRect.right() - 100, 20, QPen(Qt::blue, 2));
        scene->addText("F(x)", labelFont)->setPos(viewRect.right() - 95, 15);
        
        // Линия и текст для сплайна
        QPen dashPen(Qt::red, 2, Qt::DashLine);
        scene->addLine(viewRect.right() - 140, 40, viewRect.right() - 100, 40, dashPen);
        scene->addText("S(x)", labelFont)->setPos(viewRect.right() - 95, 35);
    }

    // Устанавливаем прямоугольник сцены так, чтобы охватить нарисованные элементы с небольшим запасом
    scene->setSceneRect(0, 0, viewRect.width(), viewRect.height());
}

QPointF MainWindow::scaleToView(double x, double y, QGraphicsView *view,
                                double minX, double maxX, double minY, double maxY)
{
    if (!view) return QPointF();

    QRectF viewRect = view->rect();
    qreal viewWidth = viewRect.width() - 20; // Постоянный отступ
    qreal viewHeight = viewRect.height() - 20;

    double scaleX = (maxX - minX == 0) ? 1 : viewWidth / (maxX - minX);
    double scaleY = (maxY - minY == 0) ? 1 : viewHeight / (maxY - minY);

    qreal viewX = 10 + (x - minX) * scaleX;
    qreal viewY = viewRect.bottom() - 10 - (y - minY) * scaleY; // Y инвертирована в QGraphicsView

    return QPointF(viewX, viewY);
}

void MainWindow::drawCurve(QGraphicsScene* scene, QGraphicsView* view,
                           const std::vector<double>& x, const std::vector<double>& y,
                           double minX, double maxX, double minY, double maxY,
                           const QPen& pen)
{
    if (!scene || !view || x.size() != y.size() || x.empty()) return;

    QPainterPath path;
    bool firstPoint = true;

    for (size_t i = 0; i < x.size(); ++i) {
        if (std::isnan(x[i]) || std::isnan(y[i]) || std::isinf(x[i]) || std::isinf(y[i])) {
            firstPoint = true; // Начать новый сегмент после NaN/inf
            continue;
        }

        QPointF viewPoint = scaleToView(x[i], y[i], view, minX, maxX, minY, maxY);

        if (firstPoint) {
            path.moveTo(viewPoint);
            firstPoint = false;
        } else {
            path.lineTo(viewPoint);
        }
    }

    scene->addPath(path, pen);
}

void MainWindow::drawPoints(QGraphicsScene* scene, QGraphicsView* view,
                            const std::vector<double>& x, const std::vector<double>& y,
                            double minX, double maxX, double minY, double maxY,
                            const QBrush& brush, double radius)
{
    if (!scene || !view || x.size() != y.size() || x.empty()) return;

    for (size_t i = 0; i < x.size(); ++i) {
        if (std::isnan(x[i]) || std::isnan(y[i]) || std::isinf(x[i]) || std::isinf(y[i])) {
            continue; // Пропустить NaN/inf точки
        }
        QPointF viewPoint = scaleToView(x[i], y[i], view, minX, maxX, minY, maxY);
        scene->addEllipse(viewPoint.x() - radius, viewPoint.y() - radius, radius * 2, radius * 2, QPen(Qt::NoPen), brush);
    }
}

// --- Реализации методов TableManager ---

void MainWindow::fillTables() {
    if (!solver) {
        // Очистить таблицы, если нет данных от solver
        if(splineValuesTable) splineValuesTable->clearContents();
        if(derivativeValuesTable) derivativeValuesTable->clearContents();
        if(coefficientTable) coefficientTable->clearContents();
        return;
    }
    fillSplineValuesTable();
    fillDerivativeValuesTable();
    fillCoefficientTable();
}

QTableWidgetItem* MainWindow::createTableItem(double value) {
    return std::isnan(value) || std::isinf(value) ?
           new QTableWidgetItem("-") :
           new QTableWidgetItem(QString::number(value, 'g', 6));
}

void MainWindow::fillSplineValuesTable() {
    if (!solver || !splineValuesTable || !useControlGridCheckbox || !controlGridNodesSpinBox) return;

    splineValuesTable->clearContents();

    std::vector<double> x_table;
    std::vector<double> f_table;
    std::vector<double> s_table;
    std::vector<double> df_table;
    std::vector<double> ds_table;

    bool useControlGrid = useControlGridCheckbox->isChecked();
    int N = useControlGrid ? controlGridNodesSpinBox->value() : 0;
    const auto& base_grid = solver->getX_nodes(); // Используем сетку узлов сплайна

    if (useControlGrid && N > 1 && !base_grid.empty()) {
        double a = base_grid.front();
        double b = base_grid.back();
        if (std::abs(b - a) > std::numeric_limits<double>::epsilon()) {
            double control_step = (b - a) / (N - 1);
            x_table.resize(N);
            f_table.resize(N);
            s_table.resize(N);
            df_table.resize(N);
            ds_table.resize(N);

            // Гарантируем точное соответствие крайних точек
            for (int i = 0; i < N; ++i) {
                if (i == 0) {
                    x_table[i] = a; // Точно a для первой точки
                } else if (i == N - 1) {
                    x_table[i] = b; // Точно b для последней точки
                } else {
                    x_table[i] = a + i * (b - a) / (N - 1);
                }
                
                f_table[i] = solver->getProblemFunc(x_table[i]);
                s_table[i] = solver->getSpline()(x_table[i]);
                df_table[i] = solver->getProblemDeriv(x_table[i]);
                ds_table[i] = solver->getSpline().ds(x_table[i]);
            }
        } else {
            useControlGrid = false;
        }
    }

    if (!useControlGrid) { // Используем базовую сетку сплайна, если контрольная сетка не используется
        x_table = solver->getX_nodes();
        f_table = solver->getF_nodes();
        s_table = solver->getS_nodes(); // S, вычисленное в узлах сплайна
        df_table = solver->getDF_nodes();
        ds_table = solver->getDS_nodes(); // S' , вычисленное в узлах сплайна
    }

    splineValuesTable->setRowCount(x_table.size());

    for (size_t i = 0; i < x_table.size(); ++i) {
        double f_s_diff = (!std::isnan(f_table[i]) && !std::isnan(s_table[i])) ? (f_table[i] - s_table[i]) : std::numeric_limits<double>::quiet_NaN();
        double df_ds_diff = (!std::isnan(df_table[i]) && !std::isnan(ds_table[i])) ? (df_table[i] - ds_table[i]) : std::numeric_limits<double>::quiet_NaN();

        splineValuesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        splineValuesTable->setItem(i, 1, createTableItem(x_table[i]));
        splineValuesTable->setItem(i, 2, createTableItem(f_table[i]));
        splineValuesTable->setItem(i, 3, createTableItem(s_table[i]));
        splineValuesTable->setItem(i, 4, createTableItem(f_s_diff));
        splineValuesTable->setItem(i, 5, createTableItem(df_table[i]));
        splineValuesTable->setItem(i, 6, createTableItem(ds_table[i]));
        splineValuesTable->setItem(i, 7, createTableItem(df_ds_diff));
    }

    splineValuesTable->resizeColumnsToContents();
}

void MainWindow::fillDerivativeValuesTable() {
    if (!solver || !derivativeValuesTable || !useControlGridCheckbox || !controlGridNodesSpinBox) return;

    derivativeValuesTable->clearContents();

    std::vector<double> x_table;
    std::vector<double> d2f_table;
    std::vector<double> d2s_table;

    bool useControlGrid = useControlGridCheckbox->isChecked();
    int N = useControlGrid ? controlGridNodesSpinBox->value() : 0;
    const auto& base_grid = solver->getX_nodes(); // Используем сетку узлов сплайна

    if (useControlGrid && N > 1 && !base_grid.empty()) {
        double a = base_grid.front();
        double b = base_grid.back();
        if (std::abs(b - a) > std::numeric_limits<double>::epsilon()) {
            double control_step = (b - a) / (N - 1);
            x_table.resize(N);
            d2f_table.resize(N);
            d2s_table.resize(N);

            // Гарантируем точное соответствие крайних точек
            for (int i = 0; i < N; ++i) {
                if (i == 0) {
                    x_table[i] = a; // Точно a для первой точки
                } else if (i == N - 1) {
                    x_table[i] = b; // Точно b для последней точки
                } else {
                    x_table[i] = a + i * control_step;
                }
                
                d2f_table[i] = solver->getProblemDeriv2(x_table[i]);
                d2s_table[i] = solver->getSpline().d2s(x_table[i]);
            }
        } else {
             useControlGrid = false;
         }
    }

    if (!useControlGrid) {
        x_table = solver->getX_nodes();
        d2f_table = solver->getD2F_nodes();
        d2s_table = solver->getD2S_nodes(); // S'' вычисленное в узлах сплайна
    }

    derivativeValuesTable->setRowCount(x_table.size());

    for (size_t i = 0; i < x_table.size(); ++i) {
        double d2f_d2s_diff = (!std::isnan(d2f_table[i]) && !std::isnan(d2s_table[i])) ? (d2f_table[i] - d2s_table[i]) : std::numeric_limits<double>::quiet_NaN();

        derivativeValuesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        derivativeValuesTable->setItem(i, 1, createTableItem(x_table[i]));
        derivativeValuesTable->setItem(i, 2, createTableItem(d2f_table[i]));
        derivativeValuesTable->setItem(i, 3, createTableItem(d2s_table[i]));
        derivativeValuesTable->setItem(i, 4, createTableItem(d2f_d2s_diff));
    }

    derivativeValuesTable->resizeColumnsToContents();
}

void MainWindow::fillCoefficientTable() {
    if (!solver || !coefficientTable) return;

    coefficientTable->clearContents();

    const auto& x = solver->getX_for_coef_table(); // Используем сетку узлов сплайна
    const auto& A = solver->getA();
    const auto& B = solver->getB();
    const auto& C = solver->getC(); // S''/2
    const auto& D = solver->getD();

    int n_intervals = A.size();
    if (x.size() < n_intervals + 1 || B.size() < n_intervals || C.size() < n_intervals + 1 || D.size() < n_intervals) {
        return; // Несоответствие данных
    }

    coefficientTable->setRowCount(n_intervals);

    for (int i = 0; i < n_intervals; ++i) {
        coefficientTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        coefficientTable->setItem(i, 1, createTableItem(x[i]));
        coefficientTable->setItem(i, 2, createTableItem(x[i+1]));
        coefficientTable->setItem(i, 3, createTableItem(A[i]));
        coefficientTable->setItem(i, 4, createTableItem(B[i]));
        coefficientTable->setItem(i, 5, createTableItem(C[i] / 2.0)); // c_i = C[i]/2
        coefficientTable->setItem(i, 6, createTableItem(D[i]));
    }

    coefficientTable->resizeColumnsToContents();
}

// --- Остальные вспомогательные методы ---
void MainWindow::updateInfoLabels() {
    if (!solver) {
        splineGridInfoLabel->setText("Сетка сплайна: -");
        controlGridInfoLabel->setText("Контрольная сетка: -");
        gridStepInfoLabel->setText("Шаг сетки: -");
        functionInfoLabel->setText("Функция: -");
        errorFunctionLabel->setText("Погр. F: -");
        errorDerivativeLabel->setText("Погр. F': -");
        errorSecondDerivativeLabel->setText("Погрешность F'': -");
        return;
    }

    int n = solver->getNodeCount();
    int N = useControlGridCheckbox->isChecked() ? controlGridNodesSpinBox->value() : n;
    double a = solver->getIntervalStart();
    double b = solver->getIntervalEnd();
    double h = solver->get_n_step(); // Используем метод для получения шага основной сетки
    double H = (N > 1) ? (b - a) / (N - 1) : 0; // Вычисляем шаг контрольной сетки
    
    MODE functionMode = solver->getFunctionMode();
    QString funcName = functionComboBox->currentText();
    
    // Информация о сетке сплайна
    splineGridInfoLabel->setText(QString("Сетка сплайна: [%1, %2], n = %3")
                                  .arg(a, 0, 'g', 4)
                                  .arg(b, 0, 'g', 4)
                                  .arg(n));
    
    // Информация о шаге основной сетки (Важная информация!) - всегда выводится
    gridStepInfoLabel->setText(QString("Шаг основной сетки:\nh = %1")
                                .arg(h, 0, 'g', 6));
    
    // Информация о контрольной сетке - выводится, только если контрольная сетка активна
    if (useControlGridCheckbox->isChecked()) {
        controlGridInfoLabel->setText(QString("Контрольная сетка:\nN = %1, H = %2")
                                      .arg(N)
                                      .arg(H, 0, 'g', 6));
        controlGridInfoLabel->setVisible(true);
    } else {
        controlGridInfoLabel->setVisible(false);
    }
    
    functionInfoLabel->setText(QString("Функция: %1").arg(funcName));

    // Вычисляем максимальные ошибки
    double maxErrF, maxErrDF, maxErrD2F;
    
    if (useControlGridCheckbox->isChecked()) {
        // Используем погрешности, вычисленные на контрольной сетке
        maxErrF = solver->getMaxErrorF_ControlGrid();
        maxErrDF = solver->getMaxErrorDF_ControlGrid();
        maxErrD2F = solver->getMaxErrorD2F_ControlGrid();
    } else {
        // Используем погрешности, вычисленные на основной сетке
        maxErrF = solver->getMaxErrorF();
        maxErrDF = solver->getMaxErrorDF();
        maxErrD2F = solver->getMaxErrorD2F();
    }

    // Отладочный вывод в консоль, чтобы проверить значения
    qDebug() << "Погрешности: F =" << maxErrF << " F' =" << maxErrDF << " F'' =" << maxErrD2F;

    QString gridType = useControlGridCheckbox->isChecked() ? "контр. сетке" : "узлах сплайна";
    
    // Форматирование погрешностей с явным указанием формата
    QString errFStr = QString::number(maxErrF, 'e', 4);
    QString errDFStr = QString::number(maxErrDF, 'e', 4);
    QString errD2FStr = QString::number(maxErrD2F, 'e', 4);
    
    // Еще одна отладочная информация
    qDebug() << "Форматированные погрешности: F =" << errFStr << " F' =" << errDFStr << " F'' =" << errD2FStr;
    
    // Напрямую задаем текст меткам, без использования .arg()
    QString fText = QString("Макс. погр. F:\n|F(x)-S(x)| = %1\n(на %2)").arg(errFStr).arg(gridType);
    QString dfText = QString("Макс. погр. F':\n|F'(x)-S'(x)| = %1\n(на %2)").arg(errDFStr).arg(gridType);
    QString d2fText = QString("Макс. погр. F'':\n|F''(x)-S''(x)| = %1\n(на %2)").arg(errD2FStr).arg(gridType);
    
    errorFunctionLabel->setText(fText);
    errorDerivativeLabel->setText(dfText);
    errorSecondDerivativeLabel->setText(d2fText);
}

// Метод для отображения справочной информации
void MainWindow::showHelpDialog() {
    QDialog* helpDialog = new QDialog(this);
    helpDialog->setWindowTitle("Справка по кубическим сплайнам");
    helpDialog->setMinimumSize(800, 600); // Увеличенный минимальный размер
    helpDialog->resize(900, 700); // Начальный размер окна больше
    
    // Устанавливаем флаги для изменения размера и максимизации окна
    helpDialog->setWindowFlags(helpDialog->windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    
    QVBoxLayout* layout = new QVBoxLayout(helpDialog);
    layout->setContentsMargins(12, 12, 12, 12); // Увеличенные отступы для лучшей читаемости
    
    QTextEdit* textEdit = new QTextEdit(helpDialog);
    textEdit->setReadOnly(true);
    
    // Устанавливаем минимальную высоту текстового поля
    textEdit->setMinimumHeight(400);
    
    // Устанавливаем политику размера для текстового поля (расширяемый)
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QString helpText = "<h2>Интерполяция кубическим сплайном</h2>"
                       "<p>Кубический сплайн - это кусочно-полиномиальная функция степени 3, проходящая через заданные точки "
                       "с соблюдением условий гладкости в узлах стыковки полиномов.</p>"
                       
                       "<h3>Математические основы</h3>"
                       "<p>Кубический сплайн S(x) на каждом интервале [x<sub>i</sub>, x<sub>i+1</sub>] задается формулой:</p>"
                       "<p>S(x) = a<sub>i</sub> + b<sub>i</sub>(x - x<sub>i</sub>) + c<sub>i</sub>(x - x<sub>i</sub>)<sup>2</sup> + d<sub>i</sub>(x - x<sub>i</sub>)<sup>3</sup></p>"
                       
                       "<p>Коэффициенты определяются из условий:</p>"
                       "<ul>"
                       "<li>S(x<sub>i</sub>) = F(x<sub>i</sub>) - значения в узлах совпадают с интерполируемой функцией</li>"
                       "<li>S(x<sub>i+1</sub>) = F(x<sub>i+1</sub>) - значения в узлах совпадают с интерполируемой функцией</li>"
                       "<li>S'(x<sub>i+0</sub>) = S'(x<sub> i-0</sub>) - непрерывность первой производной в узлах</li>"
                       "<li>S''(x<sub>i+0</sub>) = S''(x<sub>i-0</sub>) - непрерывность второй производной в узлах</li>"
                       "</ul>"
                       
                       "<h3>Граничные условия</h3>"
                       "<p>Для построения сплайна необходимо задать граничные условия на концах отрезка [a, b]:</p>"
                       "<ul>"
                       "<li><b>Естественный сплайн</b>: S''(a) = S''(b) = 0 - вторые производные на концах равны нулю</li>"
                       "<li><b>Заданные первые производные</b>: S'(a) = f'(a), S'(b) = f'(b) - задаются значения первых производных на концах</li>"
                       "<li><b>Заданные вторые производные</b>: S''(a) = f''(a), S''(b) = f''(b) - задаются значения вторых производных на концах</li>"
                       "</ul>"
                       
                       "<h3>Использование программы</h3>"
                       "<ol>"
                       "<li>Выберите интересующую функцию из выпадающего списка</li>"
                       "<li>Задайте интервал [a, b] интерполяции</li>"
                       "<li>Укажите количество узлов сплайна n (точки, через которые проходит сплайн)</li>"
                       "<li>Выберите граничные условия и при необходимости задайте их значения</li>"
                       "<li>Для более точного отображения графиков используйте контрольную сетку с большим числом точек N</li>"
                       "<li>Нажмите кнопку \"Построить сплайн\"</li>"
                       "</ol>"
                       
                       "<h3>Основные функции программы</h3>"
                       "<ul>"
                       "<li><b>Графики</b>: функция и сплайн, первая и вторая производные, погрешности</li>"
                       "<li><b>Таблицы</b>: значения функции и сплайна, коэффициенты сплайна</li>"
                       "<li><b>Статистика</b>: погрешности приближения функции и её производных</li>"
                       "</ul>"
                       
                       "<h3>Оценка погрешности</h3>"
                       "<p>Погрешность аппроксимации функции F(x) сплайном S(x) оценивается как:</p>"
                       "<ul>"
                       "<li><b>Погрешность функции</b>: max|F(x) - S(x)| - максимальная абсолютная разность между функцией и сплайном</li>"
                       "<li><b>Погрешность первой производной</b>: max|F'(x) - S'(x)| - максимальная абсолютная разность между первыми производными</li>"
                       "<li><b>Погрешность второй производной</b>: max|F''(x) - S''(x)| - максимальная абсолютная разность между вторыми производными</li>"
                       "</ul>"
                       
                       "<p>Погрешность вычисляется на контрольной сетке, которая обычно содержит больше точек, чем основная сетка сплайна.</p>"
                       
                       "<h3>Подробнее о погрешностях</h3>"
                       "<p>Теоретически, для кубического сплайна на равномерной сетке с шагом h погрешности имеют следующие порядки:</p>"
                       "<ul>"
                       "<li>||F - S|| = O(h<sup>4</sup>) - погрешность интерполяции для функции</li>"
                       "<li>||F' - S'|| = O(h<sup>3</sup>) - погрешность интерполяции для первой производной</li>"
                       "<li>||F'' - S''|| = O(h<sup>2</sup>) - погрешность интерполяции для второй производной</li>"
                       "</ul>"
                       
                       "<p>Для достижения наилучшей точности рекомендуется:</p>"
                       "<ul>"
                       "<li>Использовать достаточное количество узлов сплайна (n)</li>"
                       "<li>Выбирать граничные условия, соответствующие характеру исходной функции</li>"
                       "<li>Для плавно меняющихся функций эффективен естественный сплайн</li>"
                       "<li>Для функций с быстро меняющимися производными лучше задавать граничные условия явно</li>"
                       "</ul>"
                       
                       "<p>При исследовании погрешностей обратите внимание на следующие особенности:</p>"
                       "<ul>"
                       "<li>Максимальная погрешность может достигаться не в узлах, а между ними</li>"
                       "<li>Контрольная сетка с большим числом точек (N) позволяет точнее оценить реальную погрешность</li>"
                       "<li>Погрешность второй производной обычно имеет наибольшее значение</li>"
                       "<li>Вблизи концов отрезка [a, b] погрешность может возрастать</li>"
                       "<li>При увеличении числа узлов n в 2 раза, погрешность уменьшается примерно в 2<sup>k</sup> раз, где k - порядок сходимости (4 для функции, 3 для первой производной, 2 для второй производной)</li>"
                       "</ul>";
    
    textEdit->setHtml(helpText);
    layout->addWidget(textEdit);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* closeButton = new QPushButton("Закрыть", helpDialog);
    closeButton->setFixedWidth(100); // Фиксированная ширина кнопки для лучшего вида
    connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);
    
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
    
    // Настраиваем размеры текста для лучшей читаемости
    QFont font = textEdit->font();
    font.setPointSize(11); // Увеличенный размер шрифта
    textEdit->setFont(font);
    
    // Запускаем диалог с возможностью изменения размера
    helpDialog->setModal(true);
    helpDialog->exec();
    delete helpDialog;
}





