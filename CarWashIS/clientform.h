// clientform.h
#ifndef CLIENTFORM_H
#define CLIENTFORM_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class ClientForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientForm(QWidget *parent = nullptr);
    ~ClientForm();

    void refreshData();

private slots:
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void onSearchClient();
    void onTableDoubleClicked(const QModelIndex &index);
    void onSaveClient();
    void onCancelEdit();

private:
    void setupUI();
    void loadData();
    void clearFields();
    void setFieldsEnabled(bool enabled);

    QTableView *tableView;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    // Поля для ввода
    QLineEdit *lastNameEdit;
    QLineEdit *firstNameEdit;
    QLineEdit *middleNameEdit;
    QLineEdit *phoneEdit;
    QLineEdit *carNumberEdit;
    QLineEdit *carModelEdit;

    int currentEditId;
    bool isEditing;
};

#endif // CLIENTFORM_H