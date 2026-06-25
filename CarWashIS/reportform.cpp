// reportform.cpp
#include "reportform.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QSqlQueryModel>
#include <QTextDocument>
#include <QHeaderView>  // ← ДОБАВИТЬ

ReportForm::ReportForm(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}


ReportForm::~ReportForm() {}

void ReportForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Панель управления
    QGroupBox *controlGroup = new QGroupBox("Параметры отчёта", this);
    QGridLayout *controlLayout = new QGridLayout(controlGroup);

    controlLayout->addWidget(new QLabel("Тип отчёта:", this), 0, 0);
    reportType = new QComboBox(this);
    reportType->addItems({"Выручка за период", "Рейтинг услуг", "Все заказы"});
    reportType->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    controlLayout->addWidget(reportType, 0, 1);

    controlLayout->addWidget(new QLabel("Период с:", this), 0, 2);
    fromDate = new QDateEdit(this);
    fromDate->setDate(QDate::currentDate().addMonths(-1));
    fromDate->setCalendarPopup(true);
    fromDate->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    controlLayout->addWidget(fromDate, 0, 3);

    controlLayout->addWidget(new QLabel("по:", this), 0, 4);
    toDate = new QDateEdit(this);
    toDate->setDate(QDate::currentDate());
    toDate->setCalendarPopup(true);
    toDate->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    controlLayout->addWidget(toDate, 0, 5);

    generateButton = new QPushButton("📊 Сформировать", this);
    generateButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 5px; padding: 8px 25px; } QPushButton:hover { background-color: #2980b9; }");
    controlLayout->addWidget(generateButton, 0, 6);

    exportButton = new QPushButton("💾 CSV", this);
    exportButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 5px; padding: 8px 20px; } QPushButton:hover { background-color: #229954; }");
    controlLayout->addWidget(exportButton, 0, 7);

    printButton = new QPushButton("🖨️ Печать", this);
    printButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 8px 20px; } QPushButton:hover { background-color: #7f8c8d; }");
    controlLayout->addWidget(printButton, 0, 8);

    mainLayout->addWidget(controlGroup);

    // Информация о выручке
    QHBoxLayout *infoLayout = new QHBoxLayout();
    revenueLabel = new QLabel("💰 Общая выручка за период: 0.00 ₽", this);
    revenueLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    revenueLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(revenueLabel);
    mainLayout->addLayout(infoLayout);

    // Таблица отчёта
    reportTable = new QTableView(this);
    reportTable->setAlternatingRowColors(true);
    reportTable->horizontalHeader()->setStretchLastSection(true);
    reportTable->setStyleSheet(
        "QTableView {"
        "  border: 1px solid #bdc3c7;"
        "  border-radius: 5px;"
        "  gridline-color: #ecf0f1;"
        "  background-color: white;"
        "}"
        "QHeaderView::section {"
        "  background-color: #34495e;"
        "  color: white;"
        "  padding: 8px;"
        "  border: none;"
        "}"
        );
    mainLayout->addWidget(reportTable);

    // Connections
    connect(generateButton, &QPushButton::clicked, this, &ReportForm::generateReport);
    connect(exportButton, &QPushButton::clicked, this, &ReportForm::exportCSV);
    connect(printButton, &QPushButton::clicked, this, &ReportForm::printReport);
}

void ReportForm::refreshData()
{
    generateReport();
}

void ReportForm::generateReport()
{
    QDate from = fromDate->date();
    QDate to = toDate->date();
    int type = reportType->currentIndex();

    DatabaseManager db;

    // Очищаем старую модель, если она была
    QSqlQueryModel *oldModel = qobject_cast<QSqlQueryModel*>(reportTable->model());
    if (oldModel) {
        reportTable->setModel(nullptr);
        oldModel->deleteLater();
    }

    // Создаём новую модель с родителем this
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QString query;

    switch (type) {
    case 0: { // Выручка за период
        double revenue = db.getRevenue(from, to);
        revenueLabel->setText(QString("💰 Общая выручка за период: %1 ₽").arg(revenue, 0, 'f', 2));

        query = R"(
                SELECT
                    DATE(o.order_date) as Дата,
                    c.last_name || ' ' || c.first_name as Клиент,
                    s.name as Услуга,
                    e.last_name || ' ' || e.first_name as Исполнитель,
                    o.amount as Сумма
                FROM orders o
                JOIN clients c ON o.client_id = c.id
                JOIN services s ON o.service_id = s.id
                JOIN employees e ON o.employee_id = e.id
                WHERE o.status = 'Завершён'
                AND DATE(o.order_date) BETWEEN ? AND ?
                ORDER BY o.order_date DESC
            )";
        break;
    }
    case 1: { // Рейтинг услуг
        revenueLabel->setText("📊 Рейтинг услуг");
        query = R"(
                SELECT
                    s.name as Услуга,
                    COUNT(*) as Количество,
                    SUM(o.amount) as Выручка
                FROM orders o
                JOIN services s ON o.service_id = s.id
                WHERE o.status = 'Завершён'
                AND DATE(o.order_date) BETWEEN ? AND ?
                GROUP BY s.id
                ORDER BY Выручка DESC
            )";
        break;
    }
    case 2: { // Все заказы
        revenueLabel->setText("📋 Все заказы за период");
        query = R"(
                SELECT
                    o.id as ID,
                    DATE(o.order_date) as Дата,
                    c.last_name || ' ' || c.first_name as Клиент,
                    s.name as Услуга,
                    e.last_name || ' ' || e.first_name as Исполнитель,
                    o.status as Статус,
                    o.amount as Сумма
                FROM orders o
                JOIN clients c ON o.client_id = c.id
                JOIN services s ON o.service_id = s.id
                JOIN employees e ON o.employee_id = e.id
                WHERE DATE(o.order_date) BETWEEN ? AND ?
                ORDER BY o.order_date DESC
            )";
        break;
    }
    }

    QSqlQuery q;
    q.prepare(query);
    q.bindValue(0, from);
    q.bindValue(1, to);

    if (!q.exec()) {
        QMessageBox::critical(this, "Ошибка", q.lastError().text());
        delete model;
        return;
    }

    model->setQuery(std::move(q));
    reportTable->setModel(model);
    reportTable->resizeColumnsToContents();
}

void ReportForm::exportCSV()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "Сохранить отчёт",
                                                    QDir::homePath() + "/report.csv",
                                                    "CSV Files (*.csv)");

    if (filePath.isEmpty()) return;

    DatabaseManager db;
    if (db.exportToCSV(filePath)) {
        QMessageBox::information(this, "Успех", "Отчёт экспортирован в CSV");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось экспортировать отчёт");
    }
}

void ReportForm::printReport()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QPrintDialog::Accepted) {
        QTextDocument doc;
        QString html = "<html><body>";
        html += "<h1>Отчёт по автомойке</h1>";
        html += "<p>Период: " + fromDate->date().toString("dd.MM.yyyy") +
                " - " + toDate->date().toString("dd.MM.yyyy") + "</p>";
        html += "<hr>";

        QAbstractItemModel *model = reportTable->model();
        if (model) {
            html += "<table border='1' cellpadding='5'>";
            html += "<tr>";
            for (int i = 0; i < model->columnCount(); ++i) {
                html += "<th>" + model->headerData(i, Qt::Horizontal).toString() + "</th>";
            }
            html += "</tr>";

            for (int r = 0; r < model->rowCount(); ++r) {
                html += "<tr>";
                for (int c = 0; c < model->columnCount(); ++c) {
                    html += "<td>" + model->data(model->index(r, c)).toString() + "</td>";
                }
                html += "</tr>";
            }
            html += "</table>";
        }
        html += "</body></html>";

        doc.setHtml(html);
        doc.print(&printer);
    }
}