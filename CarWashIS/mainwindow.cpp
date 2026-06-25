// mainwindow.cpp
#include "mainwindow.h"
#include "clientform.h"
#include "orderform.h"
#include "serviceform.h"
#include "employeeform.h"
#include "reportform.h"
#include "databasemanager.h"

#include <QToolBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QIcon>
#include <QStatusBar>  // ← ДОБАВИТЬ ЭТУ СТРОКУ

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    createMenuBar();
    createToolBar();
    createStatusBar();
    applyStyle();
    updateStatusBar();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("Информационная система учёта деятельности автомоек");
    setMinimumSize(1100, 700);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    clientForm = new ClientForm(this);
    orderForm = new OrderForm(this);
    serviceForm = new ServiceForm(this);
    employeeForm = new EmployeeForm(this);
    reportForm = new ReportForm(this);

    stackedWidget->addWidget(clientForm);
    stackedWidget->addWidget(orderForm);
    stackedWidget->addWidget(serviceForm);
    stackedWidget->addWidget(employeeForm);
    stackedWidget->addWidget(reportForm);

    stackedWidget->setCurrentIndex(0);
}

void MainWindow::createMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("📁 Файл");

    QAction *exitAction = new QAction("Выход", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);

    QMenu *viewMenu = menuBar()->addMenu("👁️ Вид");
    QAction *clientAction = new QAction("Клиенты", this);
    connect(clientAction, &QAction::triggered, this, &MainWindow::switchToClients);
    viewMenu->addAction(clientAction);

    QAction *orderAction = new QAction("Заказы", this);
    connect(orderAction, &QAction::triggered, this, &MainWindow::switchToOrders);
    viewMenu->addAction(orderAction);

    QAction *serviceAction = new QAction("Услуги", this);
    connect(serviceAction, &QAction::triggered, this, &MainWindow::switchToServices);
    viewMenu->addAction(serviceAction);

    QAction *employeeAction = new QAction("Сотрудники", this);
    connect(employeeAction, &QAction::triggered, this, &MainWindow::switchToEmployees);
    viewMenu->addAction(employeeAction);

    QAction *reportAction = new QAction("Отчёты", this);
    connect(reportAction, &QAction::triggered, this, &MainWindow::switchToReports);
    viewMenu->addAction(reportAction);

    QMenu *helpMenu = menuBar()->addMenu("❓ Помощь");
    QAction *aboutAction = new QAction("О программе", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "О программе",
                           "Информационная система учёта и анализа деятельности автомоек\n"
                           "Версия 1.0.0\n"
                           "C++ / Qt 6.5 / SQLite 3\n"
                           "© Академия ТОП, 2026");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar("Навигация");
    toolBar->setMovable(false);
    toolBar->setStyleSheet("QToolBar { spacing: 5px; padding: 5px; background-color: #34495e; }");

    QAction *clientAction = new QAction("👥 Клиенты", this);
    connect(clientAction, &QAction::triggered, this, &MainWindow::switchToClients);
    toolBar->addAction(clientAction);

    QAction *orderAction = new QAction("📋 Заказы", this);
    connect(orderAction, &QAction::triggered, this, &MainWindow::switchToOrders);
    toolBar->addAction(orderAction);

    QAction *serviceAction = new QAction("⚙️ Услуги", this);
    connect(serviceAction, &QAction::triggered, this, &MainWindow::switchToServices);
    toolBar->addAction(serviceAction);

    QAction *employeeAction = new QAction("👨‍💼 Сотрудники", this);
    connect(employeeAction, &QAction::triggered, this, &MainWindow::switchToEmployees);
    toolBar->addAction(employeeAction);

    QAction *reportAction = new QAction("📊 Отчёты", this);
    connect(reportAction, &QAction::triggered, this, &MainWindow::switchToReports);
    toolBar->addAction(reportAction);

    // Разделитель
    toolBar->addSeparator();

    // Обновить данные
    QAction *refreshAction = new QAction("🔄 Обновить", this);
    connect(refreshAction, &QAction::triggered, [this]() {
        switch (stackedWidget->currentIndex()) {
        case 0: clientForm->refreshData(); break;
        case 1: orderForm->refreshData(); break;
        case 2: serviceForm->refreshData(); break;
        case 3: employeeForm->refreshData(); break;
        case 4: reportForm->refreshData(); break;
        }
        updateStatusBar();
    });
    toolBar->addAction(refreshAction);

    // Кнопка выхода
    toolBar->addSeparator();
    QAction *logoutAction = new QAction("🚪 Выход", this);
    connect(logoutAction, &QAction::triggered, [this]() {
        this->close();
    });
    toolBar->addAction(logoutAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->setStyleSheet(
        "QStatusBar { background-color: #34495e; color: white; padding: 5px; }"
        "QStatusBar QLabel { color: white; }"
        );

    statusLabel = new QLabel("Готов к работе", this);
    statusLabel->setStyleSheet("color: white;");
    statusBar()->addWidget(statusLabel);

    userLabel = new QLabel("👤 admin", this);
    userLabel->setStyleSheet("color: #bdc3c7;");
    statusBar()->addPermanentWidget(userLabel);
}

void MainWindow::applyStyle()
{
    setStyleSheet(
        "QMainWindow { background-color: #f5f6fa; }"
        "QToolBar { background-color: #34495e; }"
        "QToolBar QAction { color: white; padding: 8px 15px; }"
        "QToolBar QAction:hover { background-color: #2c3e50; border-radius: 3px; }"
        "QMenuBar { background-color: #34495e; color: white; }"
        "QMenuBar::item { padding: 5px 10px; }"
        "QMenuBar::item:selected { background-color: #2c3e50; }"
        "QMenu { background-color: #34495e; color: white; }"
        "QMenu::item:selected { background-color: #2c3e50; }"
        );
}

void MainWindow::updateStatusBar()
{
    DatabaseManager db;
    int ordersCount = db.getAllOrders().size();
    int clientsCount = db.getAllClients().size();
    statusLabel->setText(QString("📊 Всего заказов: %1 | Клиентов: %2 | Готов к работе")
                             .arg(ordersCount)
                             .arg(clientsCount));
}

void MainWindow::switchToClients()
{
    stackedWidget->setCurrentIndex(0);
    clientForm->refreshData();
    updateStatusBar();
}

void MainWindow::switchToOrders()
{
    stackedWidget->setCurrentIndex(1);
    orderForm->refreshData();
    updateStatusBar();
}

void MainWindow::switchToServices()
{
    stackedWidget->setCurrentIndex(2);
    serviceForm->refreshData();
    updateStatusBar();
}

void MainWindow::switchToEmployees()
{
    stackedWidget->setCurrentIndex(3);
    employeeForm->refreshData();
    updateStatusBar();
}

void MainWindow::switchToReports()
{
    stackedWidget->setCurrentIndex(4);
    reportForm->refreshData();
    updateStatusBar();
}