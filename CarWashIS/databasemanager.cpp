#include "databasemanager.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    dbPath = QDir::currentPath() + "/carwash.db";
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initDatabase()
{
    // Проверка существования файла БД
    bool dbExists = QFile::exists(dbPath);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        lastError = db.lastError();
        return false;
    }

    // Включение поддержки внешних ключей
    QSqlQuery pragmaQuery("PRAGMA foreign_keys = ON;");
    if (!pragmaQuery.exec()) {
        lastError = pragmaQuery.lastError();
        return false;
    }

    // Создание таблиц, если БД новая
    if (!dbExists) {
        if (!createTables()) {
            return false;
        }
        if (!createViews()) {
            return false;
        }
        if (!insertDefaultData()) {
            return false;
        }
    }

    return true;
}

bool DatabaseManager::createTables()
{
    QStringList queries;

    // Таблица клиентов
    queries << R"(
        CREATE TABLE clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            last_name TEXT NOT NULL,
            first_name TEXT NOT NULL,
            middle_name TEXT,
            phone TEXT UNIQUE NOT NULL,
            car_number TEXT NOT NULL,
            car_model TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // Таблица услуг
    queries << R"(
        CREATE TABLE services (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            price REAL NOT NULL CHECK (price >= 0)
        )
    )";

    // Таблица сотрудников
    queries << R"(
        CREATE TABLE employees (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            last_name TEXT NOT NULL,
            first_name TEXT NOT NULL,
            middle_name TEXT,
            position TEXT NOT NULL,
            schedule TEXT,
            hire_date DATE DEFAULT CURRENT_DATE
        )
    )";

    // Таблица заказов
    queries << R"(
        CREATE TABLE orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            service_id INTEGER NOT NULL,
            employee_id INTEGER NOT NULL,
            order_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT DEFAULT 'Ожидает',
            amount REAL NOT NULL CHECK (amount >= 0),
            note TEXT,
            FOREIGN KEY (client_id) REFERENCES clients(id) ON DELETE CASCADE,
            FOREIGN KEY (service_id) REFERENCES services(id) ON DELETE CASCADE,
            FOREIGN KEY (employee_id) REFERENCES employees(id) ON DELETE CASCADE
        )
    )";

    // Таблица пользователей (для авторизации)
    queries << R"(
        CREATE TABLE users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            role TEXT DEFAULT 'admin'
        )
    )";

    for (const QString &query : queries) {
        QSqlQuery q;
        if (!q.exec(query)) {
            lastError = q.lastError();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::createViews()
{
    // Представление для выручки
    QString query = R"(
        CREATE VIEW revenue_view AS
        SELECT
            strftime('%Y-%m-%d', order_date) as date,
            SUM(amount) as total_revenue,
            COUNT(*) as orders_count
        FROM orders
        WHERE status = 'Завершён'
        GROUP BY strftime('%Y-%m-%d', order_date)
    )";

    QSqlQuery q;
    if (!q.exec(query)) {
        lastError = q.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::insertDefaultData()
{
    QStringList queries;

    // Добавление тестовых данных
    queries << R"(
        INSERT INTO services (name, description, price) VALUES
        ('Стандартная мойка', 'Бесконтактная мойка кузова', 500),
        ('Мойка с полировкой', 'Мойка + полировка кузова', 1200),
        ('Химчистка салона', 'Глубокая чистка салона', 2500),
        ('Комплексная обработка', 'Мойка + полировка + химчистка', 3800)
    )";

    queries << R"(
        INSERT INTO employees (last_name, first_name, position, schedule) VALUES
        ('Иванов', 'Иван', 'Мойщик', '5/2'),
        ('Петров', 'Петр', 'Мойщик', '5/2'),
        ('Сидоров', 'Сидор', 'Старший мойщик', '5/2')
    )";

    queries << R"(
        INSERT INTO clients (last_name, first_name, phone, car_number, car_model) VALUES
        ('Смирнов', 'Алексей', '+79001234567', 'A123BC77', 'Toyota Camry'),
        ('Кузнецова', 'Мария', '+79007654321', 'B456XT77', 'Kia Rio'),
        ('Попов', 'Дмитрий', '+79009998877', 'C789MK77', 'Hyundai Solaris')
    )";

    for (const QString &query : queries) {
        QSqlQuery q;
        if (!q.exec(query)) {
            lastError = q.lastError();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::executeQuery(const QString &query, const QVariantList &params)
{
    QSqlQuery q;
    q.prepare(query);

    for (int i = 0; i < params.size(); ++i) {
        q.bindValue(i, params[i]);
    }

    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }

    return true;
}

// === КЛИЕНТЫ ===

bool DatabaseManager::addClient(const Client &client)
{
    QString query = R"(
        INSERT INTO clients (last_name, first_name, middle_name, phone, car_number, car_model)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    QVariantList params;
    params << client.lastName << client.firstName << client.middleName
           << client.phone << client.carNumber << client.carModel;

    return executeQuery(query, params);
}

bool DatabaseManager::updateClient(int id, const Client &client)
{
    QString query = R"(
        UPDATE clients SET
            last_name = ?, first_name = ?, middle_name = ?,
            phone = ?, car_number = ?, car_model = ?
        WHERE id = ?
    )";

    QVariantList params;
    params << client.lastName << client.firstName << client.middleName
           << client.phone << client.carNumber << client.carModel << id;

    return executeQuery(query, params);
}

bool DatabaseManager::deleteClient(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM clients WHERE id = ?");
    q.bindValue(0, id);

    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }

    return true;
}

QList<Client> DatabaseManager::getAllClients()
{
    return searchClients("");
}

QList<Client> DatabaseManager::searchClients(const QString &searchText)
{
    QList<Client> clients;

    QString query = R"(
        SELECT id, last_name, first_name, middle_name, phone, car_number, car_model, created_at
        FROM clients
        WHERE last_name LIKE ? OR first_name LIKE ? OR phone LIKE ? OR car_number LIKE ?
        ORDER BY last_name
    )";

    QString pattern = "%" + searchText + "%";
    QSqlQuery q;
    q.prepare(query);
    q.bindValue(0, pattern);
    q.bindValue(1, pattern);
    q.bindValue(2, pattern);
    q.bindValue(3, pattern);

    if (!q.exec()) {
        lastError = q.lastError();
        return clients;
    }

    while (q.next()) {
        Client c;
        c.id = q.value(0).toInt();
        c.lastName = q.value(1).toString();
        c.firstName = q.value(2).toString();
        c.middleName = q.value(3).toString();
        c.phone = q.value(4).toString();
        c.carNumber = q.value(5).toString();
        c.carModel = q.value(6).toString();
        c.createdAt = q.value(7).toDateTime();
        clients.append(c);
    }

    return clients;
}

Client DatabaseManager::getClientById(int id)
{
    Client c;
    c.id = -1;

    QSqlQuery q;
    q.prepare("SELECT id, last_name, first_name, middle_name, phone, car_number, car_model, created_at FROM clients WHERE id = ?");
    q.bindValue(0, id);

    if (!q.exec() || !q.next()) {
        return c;
    }

    c.id = q.value(0).toInt();
    c.lastName = q.value(1).toString();
    c.firstName = q.value(2).toString();
    c.middleName = q.value(3).toString();
    c.phone = q.value(4).toString();
    c.carNumber = q.value(5).toString();
    c.carModel = q.value(6).toString();
    c.createdAt = q.value(7).toDateTime();

    return c;
}

QString DatabaseManager::getClientName(int id)
{
    Client c = getClientById(id);
    if (c.id == -1) {
        return "Неизвестный клиент";
    }
    return c.lastName + " " + c.firstName + " " + c.middleName;
}

// === УСЛУГИ ===

bool DatabaseManager::addService(const Service &service)
{
    QString query = "INSERT INTO services (name, description, price) VALUES (?, ?, ?)";
    QVariantList params;
    params << service.name << service.description << service.price;
    return executeQuery(query, params);
}

bool DatabaseManager::updateService(int id, const Service &service)
{
    QString query = "UPDATE services SET name = ?, description = ?, price = ? WHERE id = ?";
    QVariantList params;
    params << service.name << service.description << service.price << id;
    return executeQuery(query, params);
}

bool DatabaseManager::deleteService(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM services WHERE id = ?");
    q.bindValue(0, id);
    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }
    return true;
}

QList<Service> DatabaseManager::getAllServices()
{
    QList<Service> services;

    QSqlQuery q("SELECT id, name, description, price FROM services ORDER BY name");
    if (!q.exec()) {
        lastError = q.lastError();
        return services;
    }

    while (q.next()) {
        Service s;
        s.id = q.value(0).toInt();
        s.name = q.value(1).toString();
        s.description = q.value(2).toString();
        s.price = q.value(3).toDouble();
        services.append(s);
    }

    return services;
}

Service DatabaseManager::getServiceById(int id)
{
    Service s;
    s.id = -1;

    QSqlQuery q;
    q.prepare("SELECT id, name, description, price FROM services WHERE id = ?");
    q.bindValue(0, id);

    if (!q.exec() || !q.next()) {
        return s;
    }

    s.id = q.value(0).toInt();
    s.name = q.value(1).toString();
    s.description = q.value(2).toString();
    s.price = q.value(3).toDouble();

    return s;
}

double DatabaseManager::getServicePrice(int id)
{
    Service s = getServiceById(id);
    return s.id != -1 ? s.price : 0.0;
}

QString DatabaseManager::getServiceName(int id)
{
    Service s = getServiceById(id);
    return s.id != -1 ? s.name : "Неизвестная услуга";
}

// === СОТРУДНИКИ ===

bool DatabaseManager::addEmployee(const Employee &employee)
{
    QString query = R"(
        INSERT INTO employees (last_name, first_name, middle_name, position, schedule, hire_date)
        VALUES (?, ?, ?, ?, ?, ?)
    )";
    QVariantList params;
    params << employee.lastName << employee.firstName << employee.middleName
           << employee.position << employee.schedule << employee.hireDate;
    return executeQuery(query, params);
}

bool DatabaseManager::updateEmployee(int id, const Employee &employee)
{
    QString query = R"(
        UPDATE employees SET
            last_name = ?, first_name = ?, middle_name = ?,
            position = ?, schedule = ?, hire_date = ?
        WHERE id = ?
    )";
    QVariantList params;
    params << employee.lastName << employee.firstName << employee.middleName
           << employee.position << employee.schedule << employee.hireDate << id;
    return executeQuery(query, params);
}

bool DatabaseManager::deleteEmployee(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM employees WHERE id = ?");
    q.bindValue(0, id);
    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }
    return true;
}

QList<Employee> DatabaseManager::getAllEmployees()
{
    QList<Employee> employees;

    QSqlQuery q("SELECT id, last_name, first_name, middle_name, position, schedule, hire_date FROM employees ORDER BY last_name");
    if (!q.exec()) {
        lastError = q.lastError();
        return employees;
    }

    while (q.next()) {
        Employee e;
        e.id = q.value(0).toInt();
        e.lastName = q.value(1).toString();
        e.firstName = q.value(2).toString();
        e.middleName = q.value(3).toString();
        e.position = q.value(4).toString();
        e.schedule = q.value(5).toString();
        e.hireDate = q.value(6).toDate();
        employees.append(e);
    }

    return employees;
}

Employee DatabaseManager::getEmployeeById(int id)
{
    Employee e;
    e.id = -1;

    QSqlQuery q;
    q.prepare("SELECT id, last_name, first_name, middle_name, position, schedule, hire_date FROM employees WHERE id = ?");
    q.bindValue(0, id);

    if (!q.exec() || !q.next()) {
        return e;
    }

    e.id = q.value(0).toInt();
    e.lastName = q.value(1).toString();
    e.firstName = q.value(2).toString();
    e.middleName = q.value(3).toString();
    e.position = q.value(4).toString();
    e.schedule = q.value(5).toString();
    e.hireDate = q.value(6).toDate();

    return e;
}

QString DatabaseManager::getEmployeeName(int id)
{
    Employee e = getEmployeeById(id);
    if (e.id == -1) {
        return "Неизвестный сотрудник";
    }
    return e.lastName + " " + e.firstName + " " + e.middleName;
}

// === ЗАКАЗЫ ===

bool DatabaseManager::addOrder(const Order &order)
{
    QString query = R"(
        INSERT INTO orders (client_id, service_id, employee_id, order_date, status, amount, note)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    QVariantList params;
    params << order.clientId << order.serviceId << order.employeeId
           << order.orderDate << order.status << order.amount << order.note;
    return executeQuery(query, params);
}

bool DatabaseManager::updateOrder(int id, const Order &order)
{
    QString query = R"(
        UPDATE orders SET
            client_id = ?, service_id = ?, employee_id = ?,
            order_date = ?, status = ?, amount = ?, note = ?
        WHERE id = ?
    )";
    QVariantList params;
    params << order.clientId << order.serviceId << order.employeeId
           << order.orderDate << order.status << order.amount << order.note << id;
    return executeQuery(query, params);
}

bool DatabaseManager::deleteOrder(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM orders WHERE id = ?");
    q.bindValue(0, id);
    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }
    return true;
}

bool DatabaseManager::updateOrderStatus(int id, const QString &status)
{
    QSqlQuery q;
    q.prepare("UPDATE orders SET status = ? WHERE id = ?");
    q.bindValue(0, status);
    q.bindValue(1, id);
    if (!q.exec()) {
        lastError = q.lastError();
        return false;
    }
    return true;
}

QList<Order> DatabaseManager::getAllOrders()
{
    QList<Order> orders;

    QSqlQuery q(R"(
        SELECT id, client_id, service_id, employee_id, order_date, status, amount, note
        FROM orders ORDER BY order_date DESC
    )");

    if (!q.exec()) {
        lastError = q.lastError();
        return orders;
    }

    while (q.next()) {
        Order o;
        o.id = q.value(0).toInt();
        o.clientId = q.value(1).toInt();
        o.serviceId = q.value(2).toInt();
        o.employeeId = q.value(3).toInt();
        o.orderDate = q.value(4).toDateTime();
        o.status = q.value(5).toString();
        o.amount = q.value(6).toDouble();
        o.note = q.value(7).toString();
        orders.append(o);
    }

    return orders;
}

QList<Order> DatabaseManager::getOrdersByDate(const QDate &date)
{
    return getOrdersByPeriod(date, date);
}

QList<Order> DatabaseManager::getOrdersByPeriod(const QDate &from, const QDate &to)
{
    QList<Order> orders;

    QSqlQuery q;
    q.prepare(R"(
        SELECT id, client_id, service_id, employee_id, order_date, status, amount, note
        FROM orders
        WHERE DATE(order_date) >= ? AND DATE(order_date) <= ?
        ORDER BY order_date DESC
    )");
    q.bindValue(0, from);
    q.bindValue(1, to);

    if (!q.exec()) {
        lastError = q.lastError();
        return orders;
    }

    while (q.next()) {
        Order o;
        o.id = q.value(0).toInt();
        o.clientId = q.value(1).toInt();
        o.serviceId = q.value(2).toInt();
        o.employeeId = q.value(3).toInt();
        o.orderDate = q.value(4).toDateTime();
        o.status = q.value(5).toString();
        o.amount = q.value(6).toDouble();
        o.note = q.value(7).toString();
        orders.append(o);
    }

    return orders;
}

QList<Order> DatabaseManager::getOrdersByClient(int clientId)
{
    QList<Order> orders;

    QSqlQuery q;
    q.prepare(R"(
        SELECT id, client_id, service_id, employee_id, order_date, status, amount, note
        FROM orders
        WHERE client_id = ?
        ORDER BY order_date DESC
    )");
    q.bindValue(0, clientId);

    if (!q.exec()) {
        lastError = q.lastError();
        return orders;
    }

    while (q.next()) {
        Order o;
        o.id = q.value(0).toInt();
        o.clientId = q.value(1).toInt();
        o.serviceId = q.value(2).toInt();
        o.employeeId = q.value(3).toInt();
        o.orderDate = q.value(4).toDateTime();
        o.status = q.value(5).toString();
        o.amount = q.value(6).toDouble();
        o.note = q.value(7).toString();
        orders.append(o);
    }

    return orders;
}

double DatabaseManager::getRevenue(const QDate &from, const QDate &to)
{
    QSqlQuery q;
    q.prepare(R"(
        SELECT SUM(amount) FROM orders
        WHERE status = 'Завершён'
        AND DATE(order_date) >= ? AND DATE(order_date) <= ?
    )");
    q.bindValue(0, from);
    q.bindValue(1, to);

    if (!q.exec() || !q.next()) {
        return 0.0;
    }

    return q.value(0).toDouble();
}

// === АНАЛИТИКА ===

QMap<QString, int> DatabaseManager::getOrdersByDay(const QDate &from, const QDate &to)
{
    QMap<QString, int> result;

    QSqlQuery q;
    q.prepare(R"(
        SELECT DATE(order_date) as date, COUNT(*) as count
        FROM orders
        WHERE DATE(order_date) >= ? AND DATE(order_date) <= ?
        GROUP BY DATE(order_date)
        ORDER BY date
    )");
    q.bindValue(0, from);
    q.bindValue(1, to);

    if (!q.exec()) {
        return result;
    }

    while (q.next()) {
        result[q.value(0).toString()] = q.value(1).toInt();
    }

    return result;
}

QMap<QString, double> DatabaseManager::getRevenueByService(const QDate &from, const QDate &to)
{
    QMap<QString, double> result;

    QSqlQuery q;
    q.prepare(R"(
        SELECT s.name, SUM(o.amount) as revenue
        FROM orders o
        JOIN services s ON o.service_id = s.id
        WHERE o.status = 'Завершён'
        AND DATE(o.order_date) >= ? AND DATE(o.order_date) <= ?
        GROUP BY s.id
        ORDER BY revenue DESC
    )");
    q.bindValue(0, from);
    q.bindValue(1, to);

    if (!q.exec()) {
        return result;
    }

    while (q.next()) {
        result[q.value(0).toString()] = q.value(1).toDouble();
    }

    return result;
}

// === АВТОРИЗАЦИЯ ===

bool DatabaseManager::checkLogin(const QString &username, const QString &password)
{
    QString hashedPassword = QString(QCryptographicHash::hash(
                                         password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery q;
    q.prepare("SELECT id FROM users WHERE username = ? AND password = ?");
    q.bindValue(0, username);
    q.bindValue(1, hashedPassword);

    if (!q.exec()) {
        return false;
    }

    return q.next();
}

void DatabaseManager::createDefaultAdmin()
{
    QSqlQuery q("SELECT COUNT(*) FROM users");
    if (q.exec() && q.next() && q.value(0).toInt() > 0) {
        return;
    }

    QString password = "admin123";
    QString hashedPassword = QString(QCryptographicHash::hash(
                                         password.toUtf8(), QCryptographicHash::Sha256).toHex());

    q.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
    q.bindValue(0, "admin");
    q.bindValue(1, hashedPassword);
    q.bindValue(2, "admin");
    q.exec();
}

// === ЭКСПОРТ ===

// databasemanager.cpp - функция exportToCSV
bool DatabaseManager::exportToCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << "\xEF\xBB\xBF"; // BOM для UTF-8

    stream << "ID;Клиент;Услуга;Сотрудник;Дата;Статус;Сумма;Примечание\n";

    QSqlQuery q(R"(
        SELECT
            o.id,
            c.last_name || ' ' || c.first_name as client_name,
            s.name as service_name,
            e.last_name || ' ' || e.first_name as employee_name,
            o.order_date,
            o.status,
            o.amount,
            o.note
        FROM orders o
        JOIN clients c ON o.client_id = c.id
        JOIN services s ON o.service_id = s.id
        JOIN employees e ON o.employee_id = e.id
        ORDER BY o.order_date DESC
    )");

    if (!q.exec()) {
        file.close();
        return false;
    }

    while (q.next()) {
        stream << q.value(0).toInt() << ";"
               << q.value(1).toString() << ";"
               << q.value(2).toString() << ";"
               << q.value(3).toString() << ";"
               << q.value(4).toDateTime().toString("dd.MM.yyyy HH:mm") << ";"
               << q.value(5).toString() << ";"
               << QString::number(q.value(6).toDouble(), 'f', 2) << ";"
               << q.value(7).toString() << "\n";
    }

    file.close();
    return true;
}

// === МОДЕЛИ ДЛЯ QTABLVIEW ===

QSqlTableModel* DatabaseManager::getClientModel()
{
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("clients");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}

QSqlTableModel* DatabaseManager::getOrderModel()
{
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("orders");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}

QSqlTableModel* DatabaseManager::getServiceModel()
{
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("services");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}

QSqlTableModel* DatabaseManager::getEmployeeModel()
{
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("employees");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}