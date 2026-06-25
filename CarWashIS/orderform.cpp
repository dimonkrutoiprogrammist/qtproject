// orderform.cpp
#include "orderform.h"
#include "databasemanager.h"
#include <QLabel>  // ← ИСПРАВИТЬ (было "qlabel.h")
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QHeaderView>

OrderForm::OrderForm(QWidget *parent)
    : QWidget(parent)
    , currentEditId(-1)
    , isEditing(false)
{
    setupUI();
    loadComboBoxes();
    loadData();
}

OrderForm::~OrderForm() {}

void OrderForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Поиск
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Поиск заказов...");
    searchEdit->setFixedHeight(35);
    searchEdit->setStyleSheet("padding: 5px 10px; border: 1px solid #bdc3c7; border-radius: 5px;");

    searchButton = new QPushButton("🔍 Найти", this);
    searchButton->setFixedHeight(35);
    searchButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #2980b9; }");

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    mainLayout->addLayout(searchLayout);

    // Форма
    QGroupBox *inputGroup = new QGroupBox("Данные заказа", this);
    inputGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QGridLayout *inputLayout = new QGridLayout(inputGroup);
    inputLayout->setSpacing(8);

    inputLayout->addWidget(new QLabel("Клиент:", this), 0, 0);
    clientCombo = new QComboBox(this);
    clientCombo->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(clientCombo, 0, 1);

    inputLayout->addWidget(new QLabel("Услуга:", this), 0, 2);
    serviceCombo = new QComboBox(this);
    serviceCombo->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    connect(serviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrderForm::onServiceChanged);
    inputLayout->addWidget(serviceCombo, 0, 3);

    inputLayout->addWidget(new QLabel("Исполнитель:", this), 1, 0);
    employeeCombo = new QComboBox(this);
    employeeCombo->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(employeeCombo, 1, 1);

    inputLayout->addWidget(new QLabel("Статус:", this), 1, 2);
    statusCombo = new QComboBox(this);
    statusCombo->addItems({"Ожидает", "В работе", "Завершён", "Отменён"});
    statusCombo->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(statusCombo, 1, 3);

    inputLayout->addWidget(new QLabel("Дата/Время:", this), 2, 0);
    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDisplayFormat("dd.MM.yyyy HH:mm");
    dateTimeEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(dateTimeEdit, 2, 1);

    inputLayout->addWidget(new QLabel("Сумма (₽):", this), 2, 2);
    amountEdit = new QLineEdit(this);
    amountEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(amountEdit, 2, 3);

    inputLayout->addWidget(new QLabel("Примечание:", this), 3, 0);
    noteEdit = new QTextEdit(this);
    noteEdit->setFixedHeight(50);
    noteEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(noteEdit, 3, 1, 1, 3);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("➕ Добавить", this);
    addButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #229954; }");

    editButton = new QPushButton("✏️ Редактировать", this);
    editButton->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #e67e22; }");
    editButton->setEnabled(false);

    deleteButton = new QPushButton("🗑️ Удалить", this);
    deleteButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #c0392b; }");
    deleteButton->setEnabled(false);

    statusButton = new QPushButton("🔄 Изменить статус", this);
    statusButton->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #8e44ad; }");
    statusButton->setEnabled(false);

    saveButton = new QPushButton("💾 Сохранить", this);
    saveButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #27ae60; }");
    saveButton->setVisible(false);

    cancelButton = new QPushButton("❌ Отмена", this);
    cancelButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #7f8c8d; }");
    cancelButton->setVisible(false);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(statusButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    inputLayout->addLayout(buttonLayout, 4, 0, 1, 4);

    mainLayout->addWidget(inputGroup);

    // Таблица
    tableView = new QTableView(this);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(false);
    tableView->setStyleSheet(
        "QTableView {"
        "  border: 1px solid #bdc3c7;"
        "  border-radius: 5px;"
        "  gridline-color: #ecf0f1;"
        "  background-color: white;"
        "}"
        "QTableView::item:selected {"
        "  background-color: #3498db;"
        "  color: white;"
        "}"
        "QHeaderView::section {"
        "  background-color: #34495e;"
        "  color: white;"
        "  padding: 8px;"
        "  border: none;"
        "}"
        );
    mainLayout->addWidget(tableView);

    // Connections
    connect(searchButton, &QPushButton::clicked, this, &OrderForm::onSearch);
    connect(addButton, &QPushButton::clicked, this, &OrderForm::onAddOrder);
    connect(editButton, &QPushButton::clicked, this, &OrderForm::onEditOrder);
    connect(deleteButton, &QPushButton::clicked, this, &OrderForm::onDeleteOrder);
    connect(statusButton, &QPushButton::clicked, this, &OrderForm::onUpdateStatus);
    connect(saveButton, &QPushButton::clicked, this, &OrderForm::onSaveOrder);
    connect(cancelButton, &QPushButton::clicked, this, &OrderForm::onCancelEdit);
    connect(tableView, &QTableView::doubleClicked, this, &OrderForm::onTableDoubleClicked);

    clearFields();
}

void OrderForm::loadComboBoxes()
{
    DatabaseManager db;

    // Загрузка клиентов
    clientCombo->clear();
    clientCombo->addItem("-- Выберите клиента --", -1);
    QList<Client> clients = db.getAllClients();
    for (const Client &c : clients) {
        clientCombo->addItem(c.lastName + " " + c.firstName + " (" + c.carNumber + ")", c.id);
    }

    // Загрузка услуг
    serviceCombo->clear();
    serviceCombo->addItem("-- Выберите услугу --", -1);
    QList<Service> services = db.getAllServices();
    for (const Service &s : services) {
        serviceCombo->addItem(s.name + " (" + QString::number(s.price, 'f', 2) + " ₽)", s.id);
    }

    // Загрузка сотрудников
    employeeCombo->clear();
    employeeCombo->addItem("-- Выберите исполнителя --", -1);
    QList<Employee> employees = db.getAllEmployees();
    for (const Employee &e : employees) {
        employeeCombo->addItem(e.lastName + " " + e.firstName, e.id);
    }
}

void OrderForm::loadData()
{
    DatabaseManager db;
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("orders");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Клиент");
    model->setHeaderData(2, Qt::Horizontal, "Услуга");
    model->setHeaderData(3, Qt::Horizontal, "Исполнитель");
    model->setHeaderData(4, Qt::Horizontal, "Дата");
    model->setHeaderData(5, Qt::Horizontal, "Статус");
    model->setHeaderData(6, Qt::Horizontal, "Сумма");
    model->setHeaderData(7, Qt::Horizontal, "Примечание");

    tableView->setModel(model);
    tableView->hideColumn(0);
    tableView->setColumnWidth(1, 150);
    tableView->setColumnWidth(2, 150);
    tableView->setColumnWidth(3, 150);
    tableView->setColumnWidth(4, 150);
    tableView->setColumnWidth(5, 100);
    tableView->setColumnWidth(6, 100);
    tableView->setColumnWidth(7, 150);
}

void OrderForm::refreshData()
{
    loadComboBoxes();
    loadData();
    clearFields();
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
    statusButton->setEnabled(false);
}

void OrderForm::clearFields()
{
    clientCombo->setCurrentIndex(0);
    serviceCombo->setCurrentIndex(0);
    employeeCombo->setCurrentIndex(0);
    statusCombo->setCurrentIndex(0);
    dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    amountEdit->clear();
    noteEdit->clear();
    currentEditId = -1;
    isEditing = false;
    loadComboBoxes();
}

void OrderForm::onServiceChanged(int index)
{
    Q_UNUSED(index);  // ← ДОБАВИТЬ ЭТУ СТРОКУ
    int serviceId = serviceCombo->currentData().toInt();
    if (serviceId > 0) {
        DatabaseManager db;
        double price = db.getServicePrice(serviceId);
        amountEdit->setText(QString::number(price, 'f', 2));
    }
}

void OrderForm::onAddOrder()
{
    clearFields();
    isEditing = false;
    currentEditId = -1;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    statusButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void OrderForm::onEditOrder()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите заказ для редактирования");
        return;
    }
    isEditing = true;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    statusButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void OrderForm::onDeleteOrder()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите заказ для удаления");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Вы уверены, что хотите удалить этот заказ?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DatabaseManager db;
        if (db.deleteOrder(currentEditId)) {
            QMessageBox::information(this, "Успех", "Заказ удалён");
            refreshData();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить заказ");
        }
    }
}

void OrderForm::onUpdateStatus()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите заказ");
        return;
    }

    DatabaseManager db;
    QString status = statusCombo->currentText();
    if (db.updateOrderStatus(currentEditId, status)) {
        QMessageBox::information(this, "Успех", "Статус обновлён");
        refreshData();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус");
    }
}

void OrderForm::onSaveOrder()
{
    int clientId = clientCombo->currentData().toInt();
    int serviceId = serviceCombo->currentData().toInt();
    int employeeId = employeeCombo->currentData().toInt();

    if (clientId <= 0 || serviceId <= 0 || employeeId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Заполните все обязательные поля");
        return;
    }

    Order order;
    order.clientId = clientId;
    order.serviceId = serviceId;
    order.employeeId = employeeId;
    order.orderDate = dateTimeEdit->dateTime();
    order.status = statusCombo->currentText();
    order.amount = amountEdit->text().toDouble();
    order.note = noteEdit->toPlainText();

    DatabaseManager db;
    bool success = false;

    if (isEditing) {
        success = db.updateOrder(currentEditId, order);
    } else {
        success = db.addOrder(order);
    }

    if (success) {
        QMessageBox::information(this, "Успех", isEditing ? "Заказ обновлён" : "Заказ добавлен");
        refreshData();
    } else {
        QMessageBox::critical(this, "Ошибка", db.getLastError().text());
    }
}

void OrderForm::onCancelEdit()
{
    clearFields();
    refreshData();
}

void OrderForm::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QSqlTableModel *model = qobject_cast<QSqlTableModel*>(tableView->model());
    if (!model) return;

    currentEditId = model->data(model->index(row, 0)).toInt();

    // Загрузка данных в поля
    int clientId = model->data(model->index(row, 1)).toInt();
    int serviceId = model->data(model->index(row, 2)).toInt();
    int employeeId = model->data(model->index(row, 3)).toInt();

    // Установка значений в комбобоксы
    for (int i = 0; i < clientCombo->count(); ++i) {
        if (clientCombo->itemData(i).toInt() == clientId) {
            clientCombo->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < serviceCombo->count(); ++i) {
        if (serviceCombo->itemData(i).toInt() == serviceId) {
            serviceCombo->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < employeeCombo->count(); ++i) {
        if (employeeCombo->itemData(i).toInt() == employeeId) {
            employeeCombo->setCurrentIndex(i);
            break;
        }
    }

    dateTimeEdit->setDateTime(model->data(model->index(row, 4)).toDateTime());
    statusCombo->setCurrentText(model->data(model->index(row, 5)).toString());
    amountEdit->setText(model->data(model->index(row, 6)).toString());
    noteEdit->setText(model->data(model->index(row, 7)).toString());

    editButton->setEnabled(true);
    deleteButton->setEnabled(true);
    statusButton->setEnabled(true);
}

void OrderForm::onSearch()
{
    // Простая реализация поиска
    QString text = searchEdit->text().trimmed();
    if (text.isEmpty()) {
        loadData();
        return;
    }

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("orders");
    model->setFilter(QString("id LIKE '%%1%' OR status LIKE '%%1%'").arg(text));
    model->select();

    tableView->setModel(model);
}