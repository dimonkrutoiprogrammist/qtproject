#include "employeeform.h"
#include "databasemanager.h"
#include <QLabel>  // ← ИСПРАВИТЬ (было "qlabel.h")
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QHeaderView>

EmployeeForm::EmployeeForm(QWidget *parent)
    : QWidget(parent)
    , currentEditId(-1)
    , isEditing(false)
{
    setupUI();
    loadData();
}

EmployeeForm::~EmployeeForm() {}

void EmployeeForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QGroupBox *inputGroup = new QGroupBox("Данные сотрудника", this);
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

    inputLayout->addWidget(new QLabel("Должность:", this), 1, 2);
    positionEdit = new QLineEdit(this);
    positionEdit->setPlaceholderText("Например: Мойщик");
    positionEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(positionEdit, 1, 3);

    inputLayout->addWidget(new QLabel("График:", this), 2, 0);
    scheduleEdit = new QLineEdit(this);
    scheduleEdit->setPlaceholderText("Например: 5/2");
    scheduleEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(scheduleEdit, 2, 1);

    inputLayout->addWidget(new QLabel("Дата приёма:", this), 2, 2);
    hireDateEdit = new QDateEdit(QDate::currentDate(), this);
    hireDateEdit->setCalendarPopup(true);
    hireDateEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(hireDateEdit, 2, 3);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("➕ Добавить", this);
    addButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #229954; }");

    editButton = new QPushButton("✏️ Редактировать", this);
    editButton->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #e67e22; }");
    editButton->setEnabled(false);

    deleteButton = new QPushButton("🗑️ Удалить", this);
    deleteButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #c0392b; }");
    deleteButton->setEnabled(false);

    saveButton = new QPushButton("💾 Сохранить", this);
    saveButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #27ae60; }");
    saveButton->setVisible(false);

    cancelButton = new QPushButton("❌ Отмена", this);
    cancelButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 5px 20px; } QPushButton:hover { background-color: #7f8c8d; }");
    cancelButton->setVisible(false);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    inputLayout->addLayout(buttonLayout, 3, 0, 1, 4);

    mainLayout->addWidget(inputGroup);

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

    connect(addButton, &QPushButton::clicked, this, &EmployeeForm::onAddEmployee);
    connect(editButton, &QPushButton::clicked, this, &EmployeeForm::onEditEmployee);
    connect(deleteButton, &QPushButton::clicked, this, &EmployeeForm::onDeleteEmployee);
    connect(saveButton, &QPushButton::clicked, this, &EmployeeForm::onSaveEmployee);
    connect(cancelButton, &QPushButton::clicked, this, &EmployeeForm::onCancelEdit);
    connect(tableView, &QTableView::doubleClicked, this, &EmployeeForm::onTableDoubleClicked);

    clearFields();
}

void EmployeeForm::loadData()
{
    DatabaseManager db;
    QSqlTableModel *model = db.getEmployeeModel();

    if (model) {
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Фамилия");
        model->setHeaderData(2, Qt::Horizontal, "Имя");
        model->setHeaderData(3, Qt::Horizontal, "Отчество");
        model->setHeaderData(4, Qt::Horizontal, "Должность");
        model->setHeaderData(5, Qt::Horizontal, "График");
        model->setHeaderData(6, Qt::Horizontal, "Дата приёма");

        tableView->setModel(model);
        tableView->hideColumn(0);
        tableView->setColumnWidth(1, 120);
        tableView->setColumnWidth(2, 120);
        tableView->setColumnWidth(3, 120);
        tableView->setColumnWidth(4, 150);
        tableView->setColumnWidth(5, 100);
        tableView->setColumnWidth(6, 120);
    }
}

void EmployeeForm::refreshData()
{
    loadData();
    clearFields();
}

void EmployeeForm::clearFields()
{
    lastNameEdit->clear();
    firstNameEdit->clear();
    middleNameEdit->clear();
    positionEdit->clear();
    scheduleEdit->clear();
    hireDateEdit->setDate(QDate::currentDate());
    currentEditId = -1;
    isEditing = false;

    addButton->setVisible(true);
    editButton->setVisible(true);
    deleteButton->setVisible(true);
    saveButton->setVisible(false);
    cancelButton->setVisible(false);

    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void EmployeeForm::onAddEmployee()
{
    clearFields();
    isEditing = false;
    currentEditId = -1;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);

    lastNameEdit->setFocus();
}

void EmployeeForm::onEditEmployee()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите сотрудника для редактирования");
        return;
    }
    isEditing = true;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void EmployeeForm::onDeleteEmployee()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите сотрудника для удаления");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Вы уверены, что хотите удалить этого сотрудника?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DatabaseManager db;
        if (db.deleteEmployee(currentEditId)) {
            QMessageBox::information(this, "Успех", "Сотрудник удалён");
            refreshData();
        } else {
            QMessageBox::critical(this, "Ошибка", db.getLastError().text());
        }
    }
}

void EmployeeForm::onSaveEmployee()
{
    if (lastNameEdit->text().isEmpty() || firstNameEdit->text().isEmpty() ||
        positionEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все обязательные поля");
        return;
    }

    Employee employee;
    employee.lastName = lastNameEdit->text().trimmed();
    employee.firstName = firstNameEdit->text().trimmed();
    employee.middleName = middleNameEdit->text().trimmed();
    employee.position = positionEdit->text().trimmed();
    employee.schedule = scheduleEdit->text().trimmed();
    employee.hireDate = hireDateEdit->date();

    DatabaseManager db;
    bool success = false;

    if (isEditing) {
        success = db.updateEmployee(currentEditId, employee);
    } else {
        success = db.addEmployee(employee);
    }

    if (success) {
        QMessageBox::information(this, "Успех", isEditing ? "Сотрудник обновлён" : "Сотрудник добавлен");
        refreshData();
    } else {
        QMessageBox::critical(this, "Ошибка", db.getLastError().text());
    }
}

void EmployeeForm::onCancelEdit()
{
    clearFields();
    refreshData();
}

void EmployeeForm::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QSqlTableModel *model = qobject_cast<QSqlTableModel*>(tableView->model());
    if (!model) return;

    currentEditId = model->data(model->index(row, 0)).toInt();
    lastNameEdit->setText(model->data(model->index(row, 1)).toString());
    firstNameEdit->setText(model->data(model->index(row, 2)).toString());
    middleNameEdit->setText(model->data(model->index(row, 3)).toString());
    positionEdit->setText(model->data(model->index(row, 4)).toString());
    scheduleEdit->setText(model->data(model->index(row, 5)).toString());
    hireDateEdit->setDate(model->data(model->index(row, 6)).toDate());

    editButton->setEnabled(true);
    deleteButton->setEnabled(true);
}