/*
 * main.cpp - точка входа программы
 *
 * Здесь создаётся QApplication, задаются глобальные параметры приложения, загружается базовая таблица стилей и создаётся главное окно MainWindow
 *
 * Важно: main.cpp почти не содержит бизнес-логики. Его задача — подготовить среду Qt и передать управление главному окну.
 */
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSettings>
#include <QStandardPaths>

#include "MainWindow.hpp"

namespace
{
    void cleanupApplicationData()
    {
        const QString appDataPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation);

        if (!appDataPath.isEmpty())
        {
            QDir appDataDirectory(appDataPath);
            if (appDataDirectory.exists()) appDataDirectory.removeRecursively();
        }

        QSettings settings;
        settings.clear();
        settings.sync();

        // Удаляем также старые данные, которые могли остаться от предыдущей версии приложения с другим именем
        const QString oldAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            .replace("/TaskManager/Task Manager","/DesktopApp");
        if (!oldAppDataPath.isEmpty() && oldAppDataPath != appDataPath)
        {
            QDir oldDirectory(oldAppDataPath);
            if (oldDirectory.exists()) oldDirectory.removeRecursively();
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    application.setApplicationName("Task Manager");
    application.setApplicationDisplayName("Task Manager");
    application.setOrganizationName("TaskManager");

    // Специальный режим используется деинсталлятором
    // В этом режиме MainWindow вообще не создаётся, поэтому приложение не загружает tasks.json и не сохраняет настройки обратно
    if (application.arguments().contains("--cleanup-data"))
    {
        cleanupApplicationData();
        return 0;
    }

    // Приложение может работать в системном трее без видимого главного окна
    // Поэтому Qt не должен завершать приложение только потому, что все окна скрыты
    application.setQuitOnLastWindowClosed(false);
    application.setWindowIcon(QIcon(":/app.svg"));
    // Базовая светлая тема
    // MainWindow после загрузки настроек дополнительно применит выбранную тему
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) application.setStyleSheet( QString::fromUtf8(styleFile.readAll()));
    MainWindow window;
    window.show();
    return application.exec();
}

