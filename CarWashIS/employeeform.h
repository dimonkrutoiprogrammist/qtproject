// employeeform.h
#ifndef EMPLOYEEFORM_H
#define EMPLOYEEFORM_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QDateEdit>
#include <QHeaderView>
#include <QComboBox>

class EmployeeForm : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeForm(QWidget *parent = nullptr);
    ~EmployeeForm();
    void refreshData();

private slots:
    void onAddEmployee();
    void onEditEmployee();
    void onDeleteEmployee();
    void onSaveEmployee();
    void onCancelEdit();
    void onTableDoubleClicked(const QModelIndex &index);

private:
    void setupUI();
    void loadData();
    void clearFields();

    QTableView *tableView;
    QLineEdit *lastNameEdit;
    QLineEdit *firstNameEdit;
    QLineEdit *middleNameEdit;
    QLineEdit *positionEdit;
    QLineEdit *scheduleEdit;
    QDateEdit *hireDateEdit;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    int currentEditId;
    bool isEditing;
};

#endif // EMPLOYEEFORM_H