/*
 * MainWindow.hpp — объявление главного окна приложения.
 *
 * MainWindow связывает почти все части программы:
 *   UI -> TaskModel -> TaskStorage
 *        |       |
 *        |       +-> TaskDelegate (рисование карточек)
 *        +-> TaskFilterProxyModel (поиск/фильтрация/сортировка)
 *
 * Также MainWindow отвечает за настройки, системный трей, уведомления
 * и вызов диалогов добавления/редактирования задачи.
 */
#pragma once

#include <QList>
#include <QTimer>
#include <QWidget>

#include "ReminderManager.hpp"
#include "Task.hpp"
#include "TaskFilterProxyModel.hpp"
#include "TaskModel.hpp"
#include "TaskStorage.hpp"

class QAction;
class QComboBox;
class QCloseEvent;
class QLabel;
class QLineEdit;
class QListView;
class QMenu;
class QPushButton;
class QSystemTrayIcon;

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

    bool eventFilter(
        QObject *watched,
        QEvent *event
    ) override;

private:
    void addTask();
    void editTask();
    void deleteTask();
    void toggleTaskCompleted();

    void refreshTaskList();
    void resetFilters();

    void saveTasks();

    void setupTrayIcon();

    void showFromTray();
    void hideToTray();

    void openSettings();

    void showReminder(const Task &task);

    void updateOverdueTasks();

    void loadSettings();
    void saveSettings();

    void applyTheme(const QString &theme);

    QString currentTheme() const;

    Task *findTaskById(int id);

    bool isAutostartEnabled() const;

    void setAutostartEnabled(bool enabled);

    // Проверяет, является ли место клика карточкой задачи

    bool isTaskListItemClick(
        QObject *watched,
        QEvent *event
    ) const;

    QList<Task> tasks;

    int nextTaskId = 1;

    TaskStorage storage;

    TaskModel *taskModel;
    TaskFilterProxyModel *proxyModel;
    ReminderManager *reminderManager;

    QLineEdit *searchInput;
    QComboBox *statusFilter;
    QComboBox *sortMode;

    QListView *taskList;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *completeButton;
    QPushButton *resetButton;
    QPushButton *settingsButton;

    QLabel *statisticsLabel;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QAction *autostartAction;


    // Таймер обновляет OverdueRole каждую секунду: срок мож ет наступить даже
    // тогда, когда пользователь ничего не нажимает.

    QTimer overdueRefreshTimer;

    bool reallyQuit = false;

    bool minimizeToTray = true;
    bool notificationsEnabled = true;
    bool startMinimized = false;

    int notificationDuration = 10000;
};