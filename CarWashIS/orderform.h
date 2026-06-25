// orderform.h
#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QTextEdit>

class OrderForm : public QWidget
{
    Q_OBJECT
public:
    explicit OrderForm(QWidget *parent = nullptr);
    ~OrderForm();
    void refreshData();

private slots:
    void onAddOrder();
    void onEditOrder();
    void onDeleteOrder();
    void onUpdateStatus();
    void onSaveOrder();
    void onCancelEdit();
    void onTableDoubleClicked(const QModelIndex &index);
    void onServiceChanged(int index);
    void onClientSelected(int index);
    void onSearch();

private:
    void setupUI();
    void loadData();
    void clearFields();
    void loadComboBoxes();

    QTableView *tableView;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *statusButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    QComboBox *clientCombo;
    QComboBox *serviceCombo;
    QComboBox *employeeCombo;
    QComboBox *statusCombo;
    QDateTimeEdit *dateTimeEdit;
    QLineEdit *amountEdit;
    QTextEdit *noteEdit;

    int currentEditId;
    bool isEditing;
};

#endif // ORDERFORM_H