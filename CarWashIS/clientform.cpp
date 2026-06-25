// clientform.cpp
#include "clientform.h"
#include "databasemanager.h"
#include <QSqlTableModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel> 

ClientForm::ClientForm(QWidget *parent)
    : QWidget(parent)
    , currentEditId(-1)
    , isEditing(false)
{
    setupUI();
    loadData();
}

ClientForm::~ClientForm() {}

void ClientForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Панель
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Поиск по ФИО, телефону или номеру ТС...");
    searchEdit->setFixedHeight(35);
    searchEdit->setStyleSheet("padding: 5px 10px; border: 1px solid #bdc3c7; border-radius: 5px;");

    searchButton = new QPushButton("🔍 Найти", this);
    searchButton->setFixedHeight(35);
    searchButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 5px 20px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        );

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    mainLayout->addLayout(searchLayout);

    // Форма
    QGroupBox *inputGroup = new QGroupBox("Данные клиента", this);
    inputGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QGridLayout *inputLayout = new QGridLayout(inputGroup);
    inputLayout->setSpacing(10);

    inputLayout->addWidget(new QLabel("Фамилия:", this), 0, 0);
    lastNameEdit = new QLineEdit(this);
    lastNameEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(lastNameEdit, 0, 1);

    inputLayout->addWidget(new QLabel("Имя:", this), 0, 2);
    firstNameEdit = new QLineEdit(this);
    firstNameEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(firstNameEdit, 0, 3);

    inputLayout->addWidget(new QLabel("Отчество:", this), 1, 0);
    middleNameEdit = new QLineEdit(this);
    middleNameEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(middleNameEdit, 1, 1);

    inputLayout->addWidget(new QLabel("Телефон:", this), 1, 2);
    phoneEdit = new QLineEdit(this);
    phoneEdit->setPlaceholderText("+7XXXXXXXXXX");
    phoneEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(phoneEdit, 1, 3);

    inputLayout->addWidget(new QLabel("Номер ТС:", this), 2, 0);
    carNumberEdit = new QLineEdit(this);
    carNumberEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(carNumberEdit, 2, 1);

    inputLayout->addWidget(new QLabel("Марка/Модель:", this), 2, 2);
    carModelEdit = new QLineEdit(this);
    carModelEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(carModelEdit, 2, 3);

    // Кнопки формы
    QHBoxLayout *formButtonLayout = new QHBoxLayout();
    formButtonLayout->setSpacing(10);

    addButton = new QPushButton("➕ Добавить", this);
    addButton->setFixedHeight(35);
    addButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #229954; }");

    editButton = new QPushButton("✏️ Редактировать", this);
    editButton->setFixedHeight(35);
    editButton->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #e67e22; }");
    editButton->setEnabled(false);

    deleteButton = new QPushButton("🗑️ Удалить", this);
    deleteButton->setFixedHeight(35);
    deleteButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #c0392b; }");
    deleteButton->setEnabled(false);

    saveButton = new QPushButton("💾 Сохранить", this);
    saveButton->setFixedHeight(35);
    saveButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #27ae60; }");
    saveButton->setVisible(false);

    cancelButton = new QPushButton("❌ Отмена", this);
    cancelButton->setFixedHeight(35);
    cancelButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #7f8c8d; }");
    cancelButton->setVisible(false);

    formButtonLayout->addWidget(addButton);
    formButtonLayout->addWidget(editButton);
    formButtonLayout->addWidget(deleteButton);
    formButtonLayout->addWidget(saveButton);
    formButtonLayout->addWidget(cancelButton);
    formButtonLayout->addStretch();

    inputLayout->addLayout(formButtonLayout, 3, 0, 1, 4);
    mainLayout->addWidget(inputGroup);

    // Таблица
    tableView = new QTableView(this);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setHighlightSections(false);
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

    
    connect(searchButton, &QPushButton::clicked, this, &ClientForm::onSearchClient);
    connect(searchEdit, &QLineEdit::returnPressed, this, &ClientForm::onSearchClient);
    connect(addButton, &QPushButton::clicked, this, &ClientForm::onAddClient);
    connect(editButton, &QPushButton::clicked, this, &ClientForm::onEditClient);
    connect(deleteButton, &QPushButton::clicked, this, &ClientForm::onDeleteClient);
    connect(saveButton, &QPushButton::clicked, this, &ClientForm::onSaveClient);
    connect(cancelButton, &QPushButton::clicked, this, &ClientForm::onCancelEdit);
    connect(tableView, &QTableView::doubleClicked, this, &ClientForm::onTableDoubleClicked);

    clearFields();
}

void ClientForm::loadData()
{
    DatabaseManager db;
    QSqlTableModel *model = db.getClientModel();

    if (model) {
       
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Фамилия");
        model->setHeaderData(2, Qt::Horizontal, "Имя");
        model->setHeaderData(3, Qt::Horizontal, "Отчество");
        model->setHeaderData(4, Qt::Horizontal, "Телефон");
        model->setHeaderData(5, Qt::Horizontal, "Номер ТС");
        model->setHeaderData(6, Qt::Horizontal, "Марка/Модель");
        model->setHeaderData(7, Qt::Horizontal, "Дата создания");

        tableView->setModel(model);
        tableView->hideColumn(0);
        tableView->setColumnWidth(1, 120);
        tableView->setColumnWidth(2, 120);
        tableView->setColumnWidth(3, 120);
        tableView->setColumnWidth(4, 130);
        tableView->setColumnWidth(5, 120);
        tableView->setColumnWidth(6, 150);
        tableView->setColumnWidth(7, 150);
    }
}

void ClientForm::refreshData()
{
    loadData();
    clearFields();
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
    isEditing = false;
}

void ClientForm::clearFields()
{
    lastNameEdit->clear();
    firstNameEdit->clear();
    middleNameEdit->clear();
    phoneEdit->clear();
    carNumberEdit->clear();
    carModelEdit->clear();
    currentEditId = -1;
    isEditing = false;
    setFieldsEnabled(true);
    addButton->setVisible(true);
    editButton->setVisible(true);
    deleteButton->setVisible(true);
    saveButton->setVisible(false);
    cancelButton->setVisible(false);
}

void ClientForm::setFieldsEnabled(bool enabled)
{
    lastNameEdit->setEnabled(enabled);
    firstNameEdit->setEnabled(enabled);
    middleNameEdit->setEnabled(enabled);
    phoneEdit->setEnabled(enabled);
    carNumberEdit->setEnabled(enabled);
    carModelEdit->setEnabled(enabled);
}

void ClientForm::onSearchClient()
{
    QString searchText = searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        loadData();
        return;
    }

    DatabaseManager db;
    QList<Client> clients = db.searchClients(searchText);

   
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("clients");
    model->setFilter(QString("last_name LIKE '%%1%' OR first_name LIKE '%%1%' OR phone LIKE '%%1%' OR car_number LIKE '%%1%'").arg(searchText));
    model->select();

    if (model->rowCount() == 0) {
        QMessageBox::information(this, "Поиск", "Ничего не найдено");
        loadData();
        return;
    }

    tableView->setModel(model);
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void ClientForm::onAddClient()
{
    clearFields();
    isEditing = false;
    currentEditId = -1;
    setFieldsEnabled(true);
    lastNameEdit->setFocus();

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void ClientForm::onEditClient()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для редактирования");
        return;
    }

    isEditing = true;
    setFieldsEnabled(true);

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void ClientForm::onDeleteClient()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для удаления");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Вы уверены, что хотите удалить этого клиента?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DatabaseManager db;
        if (db.deleteClient(currentEditId)) {
            QMessageBox::information(this, "Успех", "Клиент удалён");
            refreshData();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить клиента");
        }
    }
}

void ClientForm::onSaveClient()
{
    // Проверка заполнения полей
    if (lastNameEdit->text().isEmpty() || firstNameEdit->text().isEmpty() ||
        phoneEdit->text().isEmpty() || carNumberEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все обязательные поля");
        return;
    }

    Client client;
    client.lastName = lastNameEdit->text().trimmed();
    client.firstName = firstNameEdit->text().trimmed();
    client.middleName = middleNameEdit->text().trimmed();
    client.phone = phoneEdit->text().trimmed();
    client.carNumber = carNumberEdit->text().trimmed();
    client.carModel = carModelEdit->text().trimmed();

    DatabaseManager db;
    bool success = false;

    if (isEditing) {
        success = db.updateClient(currentEditId, client);
    } else {
        success = db.addClient(client);
    }

    if (success) {
        QMessageBox::information(this, "Успех", isEditing ? "Клиент обновлён" : "Клиент добавлен");
        refreshData();
    } else {
        QMessageBox::critical(this, "Ошибка", db.getLastError().text());
    }
}

void ClientForm::onCancelEdit()
{
    clearFields();
    refreshData();
}

void ClientForm::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QSqlTableModel *model = qobject_cast<QSqlTableModel*>(tableView->model());
    if (!model) return;

    currentEditId = model->data(model->index(row, 0)).toInt();

    lastNameEdit->setText(model->data(model->index(row, 1)).toString());
    firstNameEdit->setText(model->data(model->index(row, 2)).toString());
    middleNameEdit->setText(model->data(model->index(row, 3)).toString());
    phoneEdit->setText(model->data(model->index(row, 4)).toString());
    carNumberEdit->setText(model->data(model->index(row, 5)).toString());
    carModelEdit->setText(model->data(model->index(row, 6)).toString());

    editButton->setEnabled(true);
    deleteButton->setEnabled(true);
}
