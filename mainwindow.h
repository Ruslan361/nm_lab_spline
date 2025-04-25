#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <vector> // Added for plot/table data
#include <QtCharts/QChartView> // Added for PlotManager functionality
#include <QtCharts/QLineSeries> // Added for PlotManager functionality
#define QT_CHARTS_USE_NAMESPACE // Added for PlotManager functionality

#include "./headers/Solver.h"
#include "./headers/Problems.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Define grid type enum
enum GridType { UNIFORM_GRID, NON_UNIFORM_GRID };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSolveButtonClicked();
    void onFunctionChanged(int index);
    void onBorderConditionChanged(int index);
    void onTabChanged(int index);
    void onDisplayOptionsChanged();
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();
    void onAutoScaleChanged(bool checked);
    void adjustControlGridNodes(int);
    void validateControlGridNodes();
    void onBorderConditionTypeChanged(int index);

private:
    Ui::MainWindow *ui;
    Solver *solver = nullptr; // Initialize solver to nullptr

    // Graphics elements (already present)
    QTabWidget *plotTabWidget;
    QGraphicsView *functionView, *derivativeView, *secondDerivativeView;
    QGraphicsView *errorView, *errorDerivativeView, *errorSecondDerivativeView;
    QGraphicsScene *functionScene, *derivativeScene, *secondDerivativeScene;
    QGraphicsScene *errorScene, *errorDerivativeScene, *errorSecondDerivativeScene;

    // Table elements (already present)
    QTabWidget *tableTabWidget;
    QTableWidget *splineValuesTable, *derivativeValuesTable, *coefficientTable;

    // Control panel elements (already present)
    QWidget *controlPanelWidget;
    QSpinBox *nodeCountSpinBox;
    QCheckBox *useControlGridCheckbox;
    QSpinBox *controlGridNodesSpinBox;
    QCheckBox *enforceGridMultipleCheckbox;
    QComboBox *functionComboBox;
    QComboBox *gridTypeComboBox;
    QComboBox *borderConditionComboBox;
    QCheckBox *showFunctionCheckbox, *showSplineCheckbox;
    QCheckBox *showControlPointsCheckbox, *showControlGridCheckbox; // Renamed from PlotManager members
    QPushButton *zoomInButton, *zoomOutButton, *resetZoomButton;
    QCheckBox *autoScaleCheckBox; // Renamed from PlotManager member
    QPushButton *solveButton;

    // New UI elements for interval and boundary conditions (already present)
    QDoubleSpinBox *intervalStartSpinBox;
    QDoubleSpinBox *intervalEndSpinBox;
    QLabel *boundaryStartLabel;
    QDoubleSpinBox *boundaryStartSpinBox;
    QLabel *boundaryEndLabel;
    QDoubleSpinBox *boundaryEndSpinBox;

    // Results panel elements (already present)
    QWidget *resultsPanel;
    QLabel *splineGridInfoLabel, *controlGridInfoLabel;
    QLabel *errorFunctionLabel, *errorDerivativeLabel, *errorSecondDerivativeLabel;
    QLabel *functionInfoLabel, *gridStepInfoLabel;

    // Helper methods for UI setup (already present)
    void setupUI();
    void createGraphicsViews();
    void createTables();
    void createControlPanel();
    void createResultsPanel();
    void layoutUI();

    // --- Merged PlotManager Methods ---
    void updatePlots();
    void zoom(double factor); // Added from PlotManager
    void resetView(); // Added from PlotManager
    void drawFunctionPlot();
    void drawDerivativePlot();
    void drawSecondDerivativePlot();
    void drawErrorPlot();
    void drawDerivativeErrorPlot();
    void drawSecondDerivativeErrorPlot();
    void findPlotBounds(const std::vector<double>& x,
                        const std::vector<double>& y1, const std::vector<double>& y2,
                        double& minX, double& maxX, double& minY, double& maxY);
    void drawAxisWithLabels(QGraphicsScene* scene, QGraphicsView* view,
                            double minX, double maxX, double minY, double maxY,
                            const QString& xLabel, const QString& yLabel);
    void drawCurve(QGraphicsScene* scene, QGraphicsView* view,
                   const std::vector<double>& x, const std::vector<double>& y,
                   double minX, double maxX, double minY, double maxY,
                   const QPen& pen);
    void drawPoints(QGraphicsScene* scene, QGraphicsView* view,
                    const std::vector<double>& x, const std::vector<double>& y,
                    double minX, double maxX, double minY, double maxY,
                    const QBrush& brush, double radius = 3.0);
    QPointF scaleToView(double x, double y, QGraphicsView *view,
                       double minX, double maxX, double minY, double maxY);

    // --- Merged TableManager Methods ---
    void fillTables();
    void fillSplineValuesTable();
    void fillDerivativeValuesTable();
    void fillCoefficientTable();
    QTableWidgetItem* createTableItem(double value); // Added from TableManager

    // --- Other Helper Methods ---
    void updateInfoLabels();
    void showHelpDialog(); // Added for help dialog functionality
    // double calculateFunctionValue(double x, MODE mode); // Removed, use solver->getProblemFunc etc.
};
