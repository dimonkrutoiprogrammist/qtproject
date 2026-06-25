#include <QApplication>
#include <QMessageBox>
#include <QSqlError>
#include "loginwindow.h"
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("ИС Автомойка");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Академия ТОП");

    // Инициализация базы данных
    DatabaseManager dbManager;
    if (!dbManager.initDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка",
                              "Не удалось инициализировать базу данных:\n" +
                                  dbManager.getLastError().text());
        return -1;
    }

    // Создание тестового администратора при первом запуске
    dbManager.createDefaultAdmin();

    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}