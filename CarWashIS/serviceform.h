// serviceform.h
#ifndef SERVICEFORM_H
#define SERVICEFORM_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

class ServiceForm : public QWidget
{
    Q_OBJECT
public:
    explicit ServiceForm(QWidget *parent = nullptr);
    ~ServiceForm();
    void refreshData();

private slots:
    void onAddService();
    void onEditService();
    void onDeleteService();
    void onSaveService();
    void onCancelEdit();
    void onTableDoubleClicked(const QModelIndex &index);

private:
    void setupUI();
    void loadData();
    void clearFields();

    QTableView *tableView;
    QLineEdit *nameEdit;
    QTextEdit *descriptionEdit;
    QLineEdit *priceEdit;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    int currentEditId;
    bool isEditing;
};

#endif // SERVICEFORM_H