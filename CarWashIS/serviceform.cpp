// serviceform.cpp
#include "serviceform.h"
#include "databasemanager.h"
#include <QLabel>  // ← ИСПРАВИТЬ (было "qlabel.h")
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QHeaderView>

ServiceForm::ServiceForm(QWidget *parent)
    : QWidget(parent)
    , currentEditId(-1)
    , isEditing(false)
{
    setupUI();
    loadData();
}

ServiceForm::~ServiceForm() {}

void ServiceForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QGroupBox *inputGroup = new QGroupBox("Информация об услуге", this);
    QGridLayout *inputLayout = new QGridLayout(inputGroup);
    inputLayout->setSpacing(10);

    inputLayout->addWidget(new QLabel("Наименование:", this), 0, 0);
    nameEdit = new QLineEdit(this);
    nameEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(nameEdit, 0, 1);

    inputLayout->addWidget(new QLabel("Стоимость (₽):", this), 0, 2);
    priceEdit = new QLineEdit(this);
    priceEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(priceEdit, 0, 3);

    inputLayout->addWidget(new QLabel("Описание:", this), 1, 0);
    descriptionEdit = new QTextEdit(this);
    descriptionEdit->setFixedHeight(60);
    descriptionEdit->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px;");
    inputLayout->addWidget(descriptionEdit, 1, 1, 1, 3);

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
    inputLayout->addLayout(buttonLayout, 2, 0, 1, 4);

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

    connect(addButton, &QPushButton::clicked, this, &ServiceForm::onAddService);
    connect(editButton, &QPushButton::clicked, this, &ServiceForm::onEditService);
    connect(deleteButton, &QPushButton::clicked, this, &ServiceForm::onDeleteService);
    connect(saveButton, &QPushButton::clicked, this, &ServiceForm::onSaveService);
    connect(cancelButton, &QPushButton::clicked, this, &ServiceForm::onCancelEdit);
    connect(tableView, &QTableView::doubleClicked, this, &ServiceForm::onTableDoubleClicked);

    clearFields();
}

void ServiceForm::loadData()
{
    DatabaseManager db;
    QSqlTableModel *model = db.getServiceModel();

    if (model) {
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Наименование");
        model->setHeaderData(2, Qt::Horizontal, "Описание");
        model->setHeaderData(3, Qt::Horizontal, "Стоимость (₽)");

        tableView->setModel(model);
        tableView->hideColumn(0);
        tableView->setColumnWidth(1, 200);
        tableView->setColumnWidth(2, 350);
        tableView->setColumnWidth(3, 120);
    }
}

void ServiceForm::refreshData()
{
    loadData();
    clearFields();
}

void ServiceForm::clearFields()
{
    nameEdit->clear();
    descriptionEdit->clear();
    priceEdit->clear();
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

void ServiceForm::onAddService()
{
    clearFields();
    isEditing = false;
    currentEditId = -1;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);

    nameEdit->setFocus();
}

void ServiceForm::onEditService()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите услугу для редактирования");
        return;
    }
    isEditing = true;

    addButton->setVisible(false);
    editButton->setVisible(false);
    deleteButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
}

void ServiceForm::onDeleteService()
{
    if (currentEditId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите услугу для удаления");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Вы уверены, что хотите удалить эту услугу?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DatabaseManager db;
        if (db.deleteService(currentEditId)) {
            QMessageBox::information(this, "Успех", "Услуга удалена");
            refreshData();
        } else {
            QMessageBox::critical(this, "Ошибка", db.getLastError().text());
        }
    }
}

void ServiceForm::onSaveService()
{
    if (nameEdit->text().isEmpty() || priceEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните наименование и стоимость");
        return;
    }

    Service service;
    service.name = nameEdit->text().trimmed();
    service.description = descriptionEdit->toPlainText().trimmed();
    service.price = priceEdit->text().toDouble();

    DatabaseManager db;
    bool success = false;

    if (isEditing) {
        success = db.updateService(currentEditId, service);
    } else {
        success = db.addService(service);
    }

    if (success) {
        QMessageBox::information(this, "Успех", isEditing ? "Услуга обновлена" : "Услуга добавлена");
        refreshData();
    } else {
        QMessageBox::critical(this, "Ошибка", db.getLastError().text());
    }
}

void ServiceForm::onCancelEdit()
{
    clearFields();
    refreshData();
}

void ServiceForm::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QSqlTableModel *model = qobject_cast<QSqlTableModel*>(tableView->model());
    if (!model) return;

    currentEditId = model->data(model->index(row, 0)).toInt();
    nameEdit->setText(model->data(model->index(row, 1)).toString());
    descriptionEdit->setText(model->data(model->index(row, 2)).toString());
    priceEdit->setText(model->data(model->index(row, 3)).toString());

    editButton->setEnabled(true);
    deleteButton->setEnabled(true);
}