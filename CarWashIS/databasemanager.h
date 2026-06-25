// databasemanager.h
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QDate>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QMap>

struct Client {
    int id;
    QString lastName;
    QString firstName;
    QString middleName;
    QString phone;
    QString carNumber;
    QString carModel;
    QDateTime createdAt;
};

struct Service {
    int id;
    QString name;
    QString description;
    double price;
};

struct Employee {
    int id;
    QString lastName;
    QString firstName;
    QString middleName;
    QString position;
    QString schedule;
    QDate hireDate;
};

struct Order {
    int id;
    int clientId;
    int serviceId;
    int employeeId;
    QDateTime orderDate;
    QString status;
    double amount;
    QString note;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initDatabase();
    QSqlError getLastError() const { return lastError; }

    // Клиенты
    bool addClient(const Client &client);
    bool updateClient(int id, const Client &client);
    bool deleteClient(int id);
    QList<Client> getAllClients();
    QList<Client> searchClients(const QString &searchText);
    Client getClientById(int id);
    QString getClientName(int id);

    // Услуги
    bool addService(const Service &service);
    bool updateService(int id, const Service &service);
    bool deleteService(int id);
    QList<Service> getAllServices();
    Service getServiceById(int id);
    double getServicePrice(int id);
    QString getServiceName(int id);

    // Сотрудники
    bool addEmployee(const Employee &employee);
    bool updateEmployee(int id, const Employee &employee);
    bool deleteEmployee(int id);
    QList<Employee> getAllEmployees();
    Employee getEmployeeById(int id);
    QString getEmployeeName(int id);

    // Заказы
    bool addOrder(const Order &order);
    bool updateOrder(int id, const Order &order);
    bool deleteOrder(int id);
    bool updateOrderStatus(int id, const QString &status);
    QList<Order> getAllOrders();
    QList<Order> getOrdersByDate(const QDate &date);
    QList<Order> getOrdersByPeriod(const QDate &from, const QDate &to);
    QList<Order> getOrdersByClient(int clientId);
    double getRevenue(const QDate &from, const QDate &to);

    // Аналитика
    QMap<QString, int> getOrdersByDay(const QDate &from, const QDate &to);
    QMap<QString, double> getRevenueByService(const QDate &from, const QDate &to);

    // Авторизация
    bool checkLogin(const QString &username, const QString &password);
    void createDefaultAdmin();

    // Экспорт
    bool exportToCSV(const QString &filePath);

    QSqlTableModel* getClientModel();
    QSqlTableModel* getOrderModel();
    QSqlTableModel* getServiceModel();
    QSqlTableModel* getEmployeeModel();

private:
    QSqlDatabase db;
    QSqlError lastError;
    QString dbPath;

    bool createTables();
    bool createViews();
    bool insertDefaultData();
    bool executeQuery(const QString &query, const QVariantList &params = QVariantList());
};

#endif // DATABASEMANAGER_H