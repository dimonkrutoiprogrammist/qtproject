// loginwindow.cpp
#include "loginwindow.h"
#include "mainwindow.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QIcon>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    applyStyle();
}

LoginWindow::~LoginWindow() {}

void LoginWindow::setupUI()
{
    setWindowTitle("Авторизация - ИС Автомойка");
    setFixedSize(400, 300);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Заголовок
    QLabel *titleLabel = new QLabel("🚗 ИС Автомойка", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel);

    QLabel *subTitleLabel = new QLabel("Вход в систему", this);
    subTitleLabel->setAlignment(Qt::AlignCenter);
    subTitleLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-bottom: 10px;");
    mainLayout->addWidget(subTitleLabel);

    // Линия разделитель
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #bdc3c7;");
    mainLayout->addWidget(line);

    // Поле логина
    QLabel *usernameLabel = new QLabel("👤 Логин:", this);
    usernameLabel->setStyleSheet("font-size: 13px;");
    mainLayout->addWidget(usernameLabel);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Введите логин");
    usernameEdit->setFixedHeight(35);
    usernameEdit->setStyleSheet("padding: 5px 10px; border: 1px solid #bdc3c7; border-radius: 5px;");
    usernameEdit->setText("admin");
    mainLayout->addWidget(usernameEdit);

    // Поле пароля
    QLabel *passwordLabel = new QLabel("🔒 Пароль:", this);
    passwordLabel->setStyleSheet("font-size: 13px;");
    mainLayout->addWidget(passwordLabel);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Введите пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(35);
    passwordEdit->setStyleSheet("padding: 5px 10px; border: 1px solid #bdc3c7; border-radius: 5px;");
    passwordEdit->setText("admin123");
    mainLayout->addWidget(passwordEdit);

    // Статус
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    mainLayout->addWidget(statusLabel);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    loginButton = new QPushButton("Войти", this);
    loginButton->setFixedHeight(40);
    loginButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 5px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1a6a9a; }"
        );

    exitButton = new QPushButton("Выход", this);
    exitButton->setFixedHeight(40);
    exitButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #95a5a6;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 5px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #7f8c8d; }"
        "QPushButton:pressed { background-color: #6c7a7a; }"
        );

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(exitButton);
    mainLayout->addLayout(buttonLayout);

    // Connect
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(exitButton, &QPushButton::clicked, this, &LoginWindow::onExitClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);

    // Установка фокуса
    usernameEdit->setFocus();
}

void LoginWindow::applyStyle()
{
    setStyleSheet(
        "QMainWindow { background-color: #f5f6fa; }"
        "QLabel { color: #2c3e50; }"
        );
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        statusLabel->setText("⚠️ Введите логин и пароль");
        return;
    }

    DatabaseManager dbManager;
    if (dbManager.checkLogin(username, password)) {
        statusLabel->setStyleSheet("color: #27ae60; font-size: 12px;");
        statusLabel->setText("✅ Успешный вход");

        MainWindow *mainWindow = new MainWindow();
        mainWindow->show();
        this->close();
    } else {
        statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
        statusLabel->setText("❌ Неверный логин или пароль");
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
}

void LoginWindow::onExitClicked()
{
    close();
}