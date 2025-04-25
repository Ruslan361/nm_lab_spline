#include "tablemanager.h"
#include <limits>
#include <cmath>
#include <QHeaderView> // For resizeColumnsToContents

TableManager::TableManager(QTableWidget* splineTable,
                           QTableWidget* derivTable,
                           QTableWidget* coeffTable,
                           QCheckBox* useCtrlGridCb,
                           QSpinBox* ctrlGridNodesSpin) // Removed parent parameter
    : // Removed QObject(parent) initializer
      m_splineValuesTable(splineTable),
      m_derivativeValuesTable(derivTable),
      m_coefficientTable(coeffTable),
      m_useControlGridCheckbox(useCtrlGridCb),
      m_controlGridNodesSpinBox(ctrlGridNodesSpin)
{}


void TableManager::setSolver(Solver* solver) {
    m_solver = solver;
}

void TableManager::fillTables() {
    if (!m_solver) return;
    fillSplineValuesTable();
    fillDerivativeValuesTable();
    fillCoefficientTable();
}

QTableWidgetItem* TableManager::createTableItem(double value) {
    return std::isnan(value) || std::isinf(value) ?
           new QTableWidgetItem("-") :
           new QTableWidgetItem(QString::number(value, 'g', 6));
}

void TableManager::fillSplineValuesTable() {
    if (!m_solver || !m_splineValuesTable) return;

    m_splineValuesTable->clearContents(); // Clear previous data

    std::vector<double> x_table;
    std::vector<double> f_table;
    std::vector<double> s_table;
    std::vector<double> df_table;
    std::vector<double> ds_table;

    // Determine which grid to use
    bool useControlGrid = m_useControlGridCheckbox && m_useControlGridCheckbox->isChecked();
    int N = useControlGrid ? (m_controlGridNodesSpinBox ? m_controlGridNodesSpinBox->value() : 0) : 0;
    const auto& base_grid = m_solver->getX();

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

            for (int i = 0; i < N; ++i) {
                double xi = a + i * control_step;
                x_table[i] = xi;
                f_table[i] = m_solver->getProblemFunc(xi); // Use getter
                s_table[i] = m_solver->getSpline()(xi);
                df_table[i] = m_solver->getProblemDeriv(xi); // Use getter
                ds_table[i] = m_solver->getSpline().ds(xi);
            }
        } else {
            useControlGrid = false; // Fallback if interval is zero
        }
    }

    if (!useControlGrid) { // Use base grid if control grid not used or invalid
        x_table = m_solver->getX();
        f_table = m_solver->getF();
        s_table = m_solver->getS();
        df_table = m_solver->getDF();
        ds_table = m_solver->getDS();
    }

    m_splineValuesTable->setRowCount(x_table.size());

    for (size_t i = 0; i < x_table.size(); ++i) {
        double f_s_diff = (!std::isnan(f_table[i]) && !std::isnan(s_table[i])) ? (f_table[i] - s_table[i]) : std::numeric_limits<double>::quiet_NaN();
        double df_ds_diff = (!std::isnan(df_table[i]) && !std::isnan(ds_table[i])) ? (df_table[i] - ds_table[i]) : std::numeric_limits<double>::quiet_NaN();

        m_splineValuesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        m_splineValuesTable->setItem(i, 1, createTableItem(x_table[i]));
        m_splineValuesTable->setItem(i, 2, createTableItem(f_table[i]));
        m_splineValuesTable->setItem(i, 3, createTableItem(s_table[i]));
        m_splineValuesTable->setItem(i, 4, createTableItem(f_s_diff));
        m_splineValuesTable->setItem(i, 5, createTableItem(df_table[i]));
        m_splineValuesTable->setItem(i, 6, createTableItem(ds_table[i]));
        m_splineValuesTable->setItem(i, 7, createTableItem(df_ds_diff));
    }

    m_splineValuesTable->resizeColumnsToContents();
}

void TableManager::fillDerivativeValuesTable() {
     if (!m_solver || !m_derivativeValuesTable) return;

    m_derivativeValuesTable->clearContents();

    std::vector<double> x_table;
    std::vector<double> d2f_table;
    std::vector<double> d2s_table;

    bool useControlGrid = m_useControlGridCheckbox && m_useControlGridCheckbox->isChecked();
    int N = useControlGrid ? (m_controlGridNodesSpinBox ? m_controlGridNodesSpinBox->value() : 0) : 0;
    const auto& base_grid = m_solver->getX();

    if (useControlGrid && N > 1 && !base_grid.empty()) {
        double a = base_grid.front();
        double b = base_grid.back();
         if (std::abs(b - a) > std::numeric_limits<double>::epsilon()) {
            double control_step = (b - a) / (N - 1);
            x_table.resize(N);
            d2f_table.resize(N);
            d2s_table.resize(N);

            for (int i = 0; i < N; ++i) {
                double xi = a + i * control_step;
                x_table[i] = xi;
                d2f_table[i] = m_solver->getProblemDeriv2(xi); // Use getter
                d2s_table[i] = m_solver->getSpline().d2s(xi);
            }
        } else {
             useControlGrid = false;
         }
    }

    if (!useControlGrid) {
        x_table = m_solver->getX();
        d2f_table = m_solver->getD2F();
        d2s_table = m_solver->getD2S();
    }

    m_derivativeValuesTable->setRowCount(x_table.size());

    for (size_t i = 0; i < x_table.size(); ++i) {
        double d2f_d2s_diff = (!std::isnan(d2f_table[i]) && !std::isnan(d2s_table[i])) ? (d2f_table[i] - d2s_table[i]) : std::numeric_limits<double>::quiet_NaN();

        m_derivativeValuesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        m_derivativeValuesTable->setItem(i, 1, createTableItem(x_table[i]));
        m_derivativeValuesTable->setItem(i, 2, createTableItem(d2f_table[i]));
        m_derivativeValuesTable->setItem(i, 3, createTableItem(d2s_table[i]));
        m_derivativeValuesTable->setItem(i, 4, createTableItem(d2f_d2s_diff));
    }

    m_derivativeValuesTable->resizeColumnsToContents();
}

void TableManager::fillCoefficientTable() {
    if (!m_solver || !m_coefficientTable) return;

    m_coefficientTable->clearContents();

    const auto& x = m_solver->getX_for_coef_table(); // Use appropriate getter
    const auto& A = m_solver->getA();
    const auto& B = m_solver->getB();
    const auto& C = m_solver->getC(); // Note: This is S''/2 for polynomial
    const auto& D = m_solver->getD();

    int n_intervals = A.size(); // Number of intervals based on coefficients
    if (x.size() < n_intervals + 1 || B.size() < n_intervals || C.size() < n_intervals + 1 || D.size() < n_intervals) {
        // Data size mismatch, cannot fill table reliably
        return;
    }


    m_coefficientTable->setRowCount(n_intervals);

    for (int i = 0; i < n_intervals; ++i) {
        m_coefficientTable->setItem(i, 0, new QTableWidgetItem(QString::number(i))); // Interval index
        m_coefficientTable->setItem(i, 1, createTableItem(x[i])); // x_i
        m_coefficientTable->setItem(i, 2, createTableItem(x[i+1])); // x_{i+1}
        m_coefficientTable->setItem(i, 3, createTableItem(A[i])); // a_i = F(x_i)
        m_coefficientTable->setItem(i, 4, createTableItem(B[i])); // b_i
        // Display C[i]/2 as the quadratic coefficient of the polynomial S(x)
        m_coefficientTable->setItem(i, 5, createTableItem(C[i] / 2.0)); // c_i = C[i]/2
        m_coefficientTable->setItem(i, 6, createTableItem(D[i])); // d_i
    }

    m_coefficientTable->resizeColumnsToContents();
}
