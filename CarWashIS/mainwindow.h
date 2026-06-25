// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>

class ClientForm;
class OrderForm;
class ServiceForm;
class EmployeeForm;
class ReportForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void switchToClients();
    void switchToOrders();
    void switchToServices();
    void switchToEmployees();
    void switchToReports();
    void updateStatusBar();

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void applyStyle();

    QStackedWidget *stackedWidget;
    ClientForm *clientForm;
    OrderForm *orderForm;
    ServiceForm *serviceForm;
    EmployeeForm *employeeForm;
    ReportForm *reportForm;

    QLabel *statusLabel;
    QLabel *userLabel;
};

#endif // MAINWINDOW_H