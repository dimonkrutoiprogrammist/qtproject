// reportform.h
#ifndef REPORTFORM_H
#define REPORTFORM_H

#include <QWidget>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTableView>
#include <QTextEdit>

class ReportForm : public QWidget
{
    Q_OBJECT

public:
    explicit ReportForm(QWidget *parent = nullptr);
    ~ReportForm();

    void refreshData();

private slots:
    void generateReport();
    void exportCSV();
    void printReport();

private:
    void setupUI();
    void loadReport();

    QDateEdit *fromDate;
    QDateEdit *toDate;
    QComboBox *reportType;
    QPushButton *generateButton;
    QPushButton *exportButton;
    QPushButton *printButton;
    QTextEdit *reportText;
    QTableView *reportTable;
    QLabel *revenueLabel;
};

#endif // REPORTFORM_H