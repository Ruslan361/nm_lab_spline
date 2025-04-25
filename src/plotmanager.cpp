#include "plotmanager.h"
#include "Solver.h" // Add include for Solver definition
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPen>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPainterPath>
#include <limits>
#include <cmath>
#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <algorithm> // For std::min_element, std::max_element

PlotManager::PlotManager(QGraphicsView* funcView, QGraphicsScene* funcScene,
                         QGraphicsView* derivView, QGraphicsScene* derivScene,
                         QGraphicsView* secDerivView, QGraphicsScene* secDerivScene,
                         QGraphicsView* errView, QGraphicsScene* errScene,
                         QGraphicsView* errDerivView, QGraphicsScene* errDerivScene,
                         QGraphicsView* errSecDerivView, QGraphicsScene* errSecDerivScene,
                         QCheckBox* showFuncCb, QCheckBox* showSplineCb,
                         QCheckBox* showPointsCb, QCheckBox* showCtrlGridCb,
                         QCheckBox* autoScaleCb)
    : m_functionView(funcView), m_functionScene(funcScene),
      m_derivativeView(derivView), m_derivativeScene(derivScene),
      m_secondDerivativeView(secDerivView), m_secondDerivativeScene(secDerivScene),
      m_errorView(errView), m_errorScene(errScene),
      m_errorDerivativeView(errDerivView), m_errorDerivativeScene(errDerivScene),
      m_errorSecondDerivativeView(errSecDerivView), m_errorSecondDerivativeScene(errSecDerivScene),
      m_showFunctionCheckbox(showFuncCb), m_showSplineCheckbox(showSplineCb),
      m_showControlPointsCheckbox(showPointsCb), m_showControlGridCheckbox(showCtrlGridCb),
      m_autoScaleCheckBox(autoScaleCb)
{}



void PlotManager::setSolver(Solver* solver) {
    m_solver = solver;
}

void PlotManager::updatePlots() {
    if (!m_solver) return;

    drawFunctionPlot();
    drawDerivativePlot();
    drawSecondDerivativePlot();
    drawErrorPlot();
    drawDerivativeErrorPlot();
    drawSecondDerivativeErrorPlot();
}

void PlotManager::zoom(double factor) {
    // Determine current view based on some logic (e.g., active tab)
    // For simplicity, let's assume functionView for now
    if (m_functionView) m_functionView->scale(factor, factor);
    if (m_derivativeView) m_derivativeView->scale(factor, factor);
    if (m_secondDerivativeView) m_secondDerivativeView->scale(factor, factor);
    if (m_errorView) m_errorView->scale(factor, factor);
    if (m_errorDerivativeView) m_errorDerivativeView->scale(factor, factor);
    if (m_errorSecondDerivativeView) m_errorSecondDerivativeView->scale(factor, factor);
}

void PlotManager::resetView() {
    if (m_functionView) m_functionView->resetTransform();
    if (m_derivativeView) m_derivativeView->resetTransform();
    if (m_secondDerivativeView) m_secondDerivativeView->resetTransform();
    if (m_errorView) m_errorView->resetTransform();
    if (m_errorDerivativeView) m_errorDerivativeView->resetTransform();
    if (m_errorSecondDerivativeView) m_errorSecondDerivativeView->resetTransform();
    updatePlots(); // Redraw after reset
}


void PlotManager::findPlotBounds(const std::vector<double>& x,
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

    // Handle cases where all values are NaN or infinite, or only one point
    if (minY > maxY) {
        minY = -1.0;
        maxY = 1.0;
    } else if (minY == maxY) {
        minY -= 0.5;
        maxY += 0.5;
    }

    // Add padding
    double yPadding = (maxY - minY) * 0.1;
    minY -= yPadding;
    maxY += yPadding;
    // Ensure minY and maxY are not the same after padding
    if (minY == maxY) {
        minY -= 0.5;
        maxY += 0.5;
    }
}

void PlotManager::drawFunctionPlot()
{
    if (!m_solver) return;

    m_functionScene->clear();

    const auto& x = m_solver->getX();
    const auto& f = m_solver->getF();
    const auto& s = m_solver->getS();

    if (x.empty() || f.empty() || s.empty()) return; // Use .empty() instead of .isEmpty()

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, f, s, minX, maxX, minY, maxY);
    } else {
        // Use fixed bounds or bounds from UI controls if available
        // Example fixed bounds:
        minX = x.front(); maxX = x.back(); minY = -2; maxY = 2; // Example fixed bounds
    }

    drawAxisWithLabels(m_functionScene, m_functionView, minX, maxX, minY, maxY, "x", "F(x), S(x)");

    if (m_showFunctionCheckbox->isChecked()) {
        drawCurve(m_functionScene, m_functionView, x, f, minX, maxX, minY, maxY, QPen(Qt::blue, 1));
    }
    if (m_showSplineCheckbox->isChecked()) {
        drawCurve(m_functionScene, m_functionView, x, s, minX, maxX, minY, maxY, QPen(Qt::red, 1, Qt::DashLine));
    }
    if (m_showControlPointsCheckbox->isChecked()) {
        drawPoints(m_functionScene, m_functionView, x, f, minX, maxX, minY, maxY, QBrush(Qt::black));
    }
    // Add drawing for control grid points if needed
}

void PlotManager::drawDerivativePlot()
{
    if (!m_solver) return;

    m_derivativeScene->clear();

    const auto& x = m_solver->getX();
    const auto& df = m_solver->getDF();
    const auto& ds = m_solver->getDS();

    if (x.empty() || df.empty() || ds.empty()) return; // Use .empty() instead of .isEmpty()

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, df, ds, minX, maxX, minY, maxY);
    } else {
        minX = x.front(); maxX = x.back(); minY = -5; maxY = 5; // Example fixed bounds
    }

    drawAxisWithLabels(m_derivativeScene, m_derivativeView, minX, maxX, minY, maxY, "x", "F'(x), S'(x)");

    if (m_showFunctionCheckbox->isChecked()) {
        drawCurve(m_derivativeScene, m_derivativeView, x, df, minX, maxX, minY, maxY, QPen(Qt::blue, 1));
    }
    if (m_showSplineCheckbox->isChecked()) {
        drawCurve(m_derivativeScene, m_derivativeView, x, ds, minX, maxX, minY, maxY, QPen(Qt::red, 1, Qt::DashLine));
    }
}

void PlotManager::drawSecondDerivativePlot()
{
    if (!m_solver) return;

    m_secondDerivativeScene->clear();

    const auto& x = m_solver->getX();
    const auto& d2f = m_solver->getD2F();
    const auto& d2s = m_solver->getD2S();

    if (x.empty() || d2f.empty() || d2s.empty()) return; // Use .empty() instead of .isEmpty()

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, d2f, d2s, minX, maxX, minY, maxY);
    } else {
        minX = x.front(); maxX = x.back(); minY = -10; maxY = 10; // Example fixed bounds
    }

    drawAxisWithLabels(m_secondDerivativeScene, m_secondDerivativeView, minX, maxX, minY, maxY, "x", "F''(x), S''(x)");

    if (m_showFunctionCheckbox->isChecked()) {
        drawCurve(m_secondDerivativeScene, m_secondDerivativeView, x, d2f, minX, maxX, minY, maxY, QPen(Qt::blue, 1));
    }
    if (m_showSplineCheckbox->isChecked()) {
        drawCurve(m_secondDerivativeScene, m_secondDerivativeView, x, d2s, minX, maxX, minY, maxY, QPen(Qt::red, 1, Qt::DashLine));
    }
}

void PlotManager::drawErrorPlot()
{
    if (!m_solver) return;

    m_errorScene->clear();

    const auto& x = m_solver->getX();
    const auto& f = m_solver->getF();
    const auto& s = m_solver->getS();

    if (x.size() != f.size() || x.size() != s.size() || x.empty()) return; // Use .empty() instead of .isEmpty()

    std::vector<double> error(x.size()); // Use std::vector<double>
    for (size_t i = 0; i < x.size(); ++i) { // Use size_t for indexing std::vector
        error[i] = qAbs(f[i] - s[i]);
    }

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY); // Pass empty vector for third dataset
    } else {
        // Example fixed bounds:
        minX = x.front(); maxX = x.back(); minY = 0; maxY = 1e-3; // Example fixed bounds
    }

    drawAxisWithLabels(m_errorScene, m_errorView, minX, maxX, minY, maxY, "x", "|F(x)-S(x)|");
    drawCurve(m_errorScene, m_errorView, x, error, minX, maxX, minY, maxY, QPen(Qt::darkGreen, 1));
}

void PlotManager::drawDerivativeErrorPlot()
{
    if (!m_solver) return;

    m_errorDerivativeScene->clear();

    const auto& x = m_solver->getX();
    const auto& df = m_solver->getDF();
    const auto& ds = m_solver->getDS();

    if (x.size() != df.size() || x.size() != ds.size() || x.empty()) return; // Use .empty() instead of .isEmpty()

    std::vector<double> error(x.size()); // Use std::vector<double>
    for (size_t i = 0; i < x.size(); ++i) { // Use size_t for indexing std::vector
        error[i] = qAbs(df[i] - ds[i]);
    }

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY); // Pass empty vector for third dataset
    } else {
        // Example fixed bounds:
        minX = x.front(); maxX = x.back(); minY = 0; maxY = 1e-2; // Example fixed bounds
    }

    drawAxisWithLabels(m_errorDerivativeScene, m_errorDerivativeView, minX, maxX, minY, maxY, "x", "|F'(x)-S'(x)|");
    drawCurve(m_errorDerivativeScene, m_errorDerivativeView, x, error, minX, maxX, minY, maxY, QPen(Qt::darkGreen, 1));
}

void PlotManager::drawSecondDerivativeErrorPlot()
{
    if (!m_solver) return;

    m_errorSecondDerivativeScene->clear();

    const auto& x = m_solver->getX();
    const auto& d2f = m_solver->getD2F();
    const auto& d2s = m_solver->getD2S();

    if (x.size() != d2f.size() || x.size() != d2s.size() || x.empty()) return; // Use .empty() instead of .isEmpty()

    std::vector<double> error(x.size()); // Use std::vector<double>
    for (size_t i = 0; i < x.size(); ++i) { // Use size_t for indexing std::vector
        error[i] = qAbs(d2f[i] - d2s[i]);
    }

    // Declare min/max variables
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (m_autoScaleCheckBox->isChecked()) { // Use m_autoScaleCheckBox->isChecked()
        findPlotBounds(x, error, {}, minX, maxX, minY, maxY); // Pass empty vector for third dataset
    } else {
        // Example fixed bounds:
        minX = x.front(); maxX = x.back(); minY = 0; maxY = 1e-1; // Example fixed bounds
    }

    drawAxisWithLabels(m_errorSecondDerivativeScene, m_errorSecondDerivativeView, minX, maxX, minY, maxY, "x", "|F''(x)-S''(x)|");
    drawCurve(m_errorSecondDerivativeScene, m_errorSecondDerivativeView, x, error, minX, maxX, minY, maxY, QPen(Qt::darkGreen, 1));
}


// --- Drawing Helper Implementations ---

void PlotManager::drawAxisWithLabels(QGraphicsScene* scene, QGraphicsView* view,
                                     double minX, double maxX, double minY, double maxY,
                                     const QString& xLabel, const QString& yLabel)
{
    // Implementation from MainWindow::drawAxisWithLabels
    if (!scene || !view) return;
    if (maxX <= minX || maxY <= minY) return; // Invalid bounds

    QRectF viewRect = view->rect(); // Use view's rect for scaling calculations
    qreal margin = 30; // Margin for labels

    // Scale factors
    qreal scaleX = (viewRect.width() - 2 * margin) / (maxX - minX);
    qreal scaleY = (viewRect.height() - 2 * margin) / (maxY - minY);

    // Origin in view coordinates
    qreal originX_view = margin - minX * scaleX;
    qreal originY_view = viewRect.height() - margin + minY * scaleY;

    // Clamp origin to view boundaries if axes are outside
    originX_view = std::max(margin, std::min(viewRect.width() - margin, originX_view));
    originY_view = std::max(margin, std::min(viewRect.height() - margin, originY_view));

    // Draw axes
    QPen axisPen(Qt::black, 1);
    scene->addLine(margin, originY_view, viewRect.width() - margin, originY_view, axisPen); // X-axis
    scene->addLine(originX_view, margin, originX_view, viewRect.height() - margin, axisPen); // Y-axis

    // Draw ticks and labels (simplified example)
    int numTicks = 5;
    QFont labelFont("Arial", 8);

    // X-axis ticks and labels
    for (int i = 0; i <= numTicks; ++i) {
        double xVal = minX + (maxX - minX) * i / numTicks;
        qreal xPos = margin + (xVal - minX) * scaleX;
        scene->addLine(xPos, originY_view - 3, xPos, originY_view + 3, axisPen);
        QGraphicsTextItem* label = scene->addText(QString::number(xVal, 'g', 3), labelFont);
        label->setPos(xPos - label->boundingRect().width() / 2, originY_view + 5);
    }

    // Y-axis ticks and labels
    for (int i = 0; i <= numTicks; ++i) {
        double yVal = minY + (maxY - minY) * i / numTicks;
        qreal yPos = viewRect.height() - margin - (yVal - minY) * scaleY;
        scene->addLine(originX_view - 3, yPos, originX_view + 3, yPos, axisPen);
        QGraphicsTextItem* label = scene->addText(QString::number(yVal, 'g', 3), labelFont);
        label->setPos(originX_view - label->boundingRect().width() - 5, yPos - label->boundingRect().height() / 2);
    }

    // Axis labels
    QGraphicsTextItem* xLabelItem = scene->addText(xLabel, labelFont);
    xLabelItem->setPos(viewRect.width() - margin - xLabelItem->boundingRect().width(), originY_view + 15);
    QGraphicsTextItem* yLabelItem = scene->addText(yLabel, labelFont);
    yLabelItem->setPos(originX_view - yLabelItem->boundingRect().width() - 15, margin);

    // Set scene rect slightly larger than view to avoid scrollbars initially if possible
    scene->setSceneRect(viewRect.adjusted(-5, -5, 5, 5));
}


void PlotManager::drawCurve(QGraphicsScene* scene, QGraphicsView* view,
                            const std::vector<double>& x, const std::vector<double>& y,
                            double minX, double maxX, double minY, double maxY,
                            const QPen& pen)
{
    // Implementation from MainWindow::drawCurve
    if (!scene || !view || x.size() < 2 || x.size() != y.size()) return;

    QPainterPath path;
    bool segmentStarted = false;

    for (size_t i = 0; i < x.size(); ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            segmentStarted = false; // End current segment if NaN/inf encountered
            continue;
        }

        QPointF p = scaleToView(x[i], y[i], view, minX, maxX, minY, maxY);

        if (!segmentStarted) {
            path.moveTo(p);
            segmentStarted = true;
        } else {
            path.lineTo(p);
        }
    }
    scene->addPath(path, pen);
}

void PlotManager::drawPoints(QGraphicsScene* scene, QGraphicsView* view,
                             const std::vector<double>& x, const std::vector<double>& y,
                             double minX, double maxX, double minY, double maxY,
                             const QBrush& brush, double radius)
{
    if (!scene || !view || x.empty() || x.size() != y.size()) return;

    for (size_t i = 0; i < x.size(); ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) continue;

        QPointF p = scaleToView(x[i], y[i], view, minX, maxX, minY, maxY);
        scene->addEllipse(p.x() - radius, p.y() - radius, 2 * radius, 2 * radius, QPen(Qt::NoPen), brush);
    }
}


QPointF PlotManager::scaleToView(double x, double y, QGraphicsView *view,
                                double minX, double maxX, double minY, double maxY)
{
    // Implementation from MainWindow::scaleToView
    if (!view || maxX <= minX || maxY <= minY) return QPointF();

    QRectF viewRect = view->rect();
    qreal margin = 30; // Consistent margin

    qreal scaleX = (viewRect.width() - 2 * margin) / (maxX - minX);
    qreal scaleY = (viewRect.height() - 2 * margin) / (maxY - minY);

    qreal viewX = margin + (x - minX) * scaleX;
    qreal viewY = viewRect.height() - margin - (y - minY) * scaleY; // Y is inverted

    return QPointF(viewX, viewY);
}
