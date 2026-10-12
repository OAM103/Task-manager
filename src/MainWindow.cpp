/* MainWindow.cpp  реализация главного окна
 *
 * Это центральный файл приложения. Если пользователь нажимает кнопку, открывает настройки, добавляет задачу или закрывает окно - обработка в конечном итоге происходит здесь
 * Пользователь - MainWindow - TaskModel / TaskFilterProxyModel - QListView + TaskDelegate
 * Хранение работает отдельно:  MainWindow <-> TaskStorage <-> tasks.json
 */
#include "MainWindow.hpp"

#include "SettingsDialog.hpp"
#include "TaskDelegate.hpp"
#include "TaskDialog.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QVBoxLayout>

// Создание главного окна: здесь строится UI, подключаются сигналы, загружаются данные и запускаются таймеры
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      storage(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasks.json"),
      taskModel(new TaskModel(this)),
      proxyModel(new TaskFilterProxyModel(this)),
      reminderManager(new ReminderManager(this)),
      trayIcon(new QSystemTrayIcon(this)),
      trayMenu(new QMenu(this)),
      autostartAction(nullptr)
{
    setWindowTitle("Task Manager");
    setMinimumSize(800, 550);
    setWindowIcon(QIcon(":/app.svg"));
    // Перехватываем клики внутри приложения, чтобы управлять снятием выделения.
    qApp->installEventFilter(this);
    loadSettings();
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Поиск по названию и описанию...");

    statusFilter = new QComboBox(this);
    statusFilter->addItem("Все");
    statusFilter->addItem("Активные");
    statusFilter->addItem("Выполненные");

    sortMode = new QComboBox(this);
    sortMode->addItem("Без сортировки");
    sortMode->addItem("По сроку");
    sortMode->addItem("По приоритету");

    taskList = new QListView(this);

    proxyModel->setSourceModel(taskModel);
    proxyModel->setSortRole(TaskModel::DueDateRole);

    taskList->setModel(proxyModel);
    taskList->setItemDelegate(new TaskDelegate(taskList));
    taskList->setSpacing(4);
    taskList->setSelectionMode(QAbstractItemView::SingleSelection);
    taskList->setSelectionBehavior(QAbstractItemView::SelectRows);

    addButton = new QPushButton("Добавить", this);
    editButton = new QPushButton("Редактировать", this);
    completeButton = new QPushButton("Выполнить", this);
    deleteButton = new QPushButton("Удалить", this);
    resetButton = new QPushButton("Сбросить", this);
    settingsButton = new QPushButton("⚙ Настройки", this);
    statisticsLabel = new QLabel(this);

    auto *filterLayout = new QHBoxLayout;

    filterLayout->addWidget(searchInput);
    filterLayout->addWidget(statusFilter);
    filterLayout->addWidget(sortMode);
    filterLayout->addWidget(resetButton);

    auto *buttonLayout =new QHBoxLayout;

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(completeButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(settingsButton);

    auto *layout = new QVBoxLayout(this);

    layout->addLayout(filterLayout);
    layout->addWidget(taskList);
    layout->addLayout(buttonLayout);
    layout->addWidget(statisticsLabel);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editTask);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTask);
    connect(completeButton, &QPushButton::clicked, this, &MainWindow::toggleTaskCompleted);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetFilters);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(searchInput, &QLineEdit::textChanged, this, [this](const QString &text)
        {
            proxyModel->setSearchText(text);
            refreshTaskList();
        }
    );

    connect(
        statusFilter,
        &QComboBox::currentIndexChanged,
        this,
        [this](int index)
        {
            switch (index)
            {
                case 1:
                    proxyModel->setStatusFilter(TaskFilterProxyModel::StatusFilter::Active);
                    break;
                case 2:
                    proxyModel->setStatusFilter(TaskFilterProxyModel::StatusFilter::Completed);
                    break;
                default:
                    proxyModel->setStatusFilter(TaskFilterProxyModel::StatusFilter::All);
                    break;
            }
            refreshTaskList();
        }
    );

    connect(sortMode, &QComboBox::currentIndexChanged, this, [this]()
        {
            refreshTaskList();
        }
    );

    connect(taskList, &QListView::doubleClicked, this, [this](const QModelIndex &)
        {
            editTask();
        }
    );

    connect(reminderManager, &ReminderManager::reminderTriggered, this, &MainWindow::showReminder);

    overdueRefreshTimer.setInterval(1000);

    connect(&overdueRefreshTimer, &QTimer::timeout, this, &MainWindow::updateOverdueTasks);

    overdueRefreshTimer.start();

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!storage.load(tasks))
    {
        tasks.clear();
    }

    for (const Task &task : tasks)
    {
        if (task.id >= nextTaskId) nextTaskId = task.id + 1;
    }

    taskModel->setTasks(tasks);
    reminderManager->setTasks(tasks);

    setupTrayIcon();
    applyTheme(currentTheme());
    refreshTaskList();

    if (startMinimized) hideToTray();
}

// Деструктор сохраняет задачи/настройки и снимает глобальный event filter
MainWindow::~MainWindow()
{
    // Убираем обработчик событий перед уничтожением MainWindow
    qApp->removeEventFilter(this);

    saveTasks();
    saveSettings();
}

bool MainWindow::isTaskListItemClick(QObject *watched, QEvent *event) const
{
    if (!taskList) return false;
    if (event->type() != QEvent::MouseButtonPress) return false;
    if (watched != taskList && watched != taskList->viewport()) return false;

    auto *mouseEvent = static_cast<QMouseEvent *>(event);

    QPoint position = mouseEvent->position().toPoint();

    // Для viewport координаты уже подходят
    // Для самого QListView преобразуем координаты в координаты viewport
    if (watched == taskList) position = taskList->viewport()->mapFrom(taskList, position);

    const QModelIndex index = taskList->indexAt(position);

    return index.isValid();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        // Если кликнули непосредственно по карточке задачи, ничего не делаем
        // QListView самостоятельно установит новое выделение
        if (isTaskListItemClick(watched, event)) return QWidget::eventFilter(watched, event);

        // Любой другой клик мышью снимает выделение
        if (taskList && taskList->selectionModel()) taskList->clearSelection();
    }

    return QWidget::eventFilter(watched, event);
}

// Читаем пользовательские настройки через QSettings. Значения по умолчанию используются при первом запуске
void MainWindow::loadSettings()
{
    QSettings settings;

    minimizeToTray = settings.value("behavior/minimizeToTray", true).toBool();
    notificationsEnabled = settings.value("notifications/enabled", true).toBool();
    notificationDuration = settings.value("notifications/duration", 10000).toInt();
    startMinimized = settings.value("behavior/startMinimized", false).toBool();
}

// Записываем настройки. sync() просит QSettings немедленно синхронизировать данные с хранилищем ОС
void MainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue("behavior/minimizeToTray", minimizeToTray);
    settings.setValue("behavior/startMinimized", startMinimized);
    settings.setValue("notifications/enabled", notificationsEnabled);
    settings.setValue("notifications/duration", notificationDuration);
    settings.sync();
}

QString MainWindow::currentTheme() const
{
    QSettings settings;

    QString theme = settings.value("appearance/theme", "light").toString();
    if (theme != "light" && theme != "dark") theme = "light";

    return theme;
}

// Загружаем выбранный QSS из Qt Resource System и применяем его ко всему приложению
void MainWindow::applyTheme(const QString &theme)
{
    const QString resourcePath = theme == "dark" ? ":/dark.qss" : ":/style.qss";

    QFile file(resourcePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))return;

    QTextStream stream(&file);

    qApp->setStyleSheet(stream.readAll());

    if (taskList) taskList->viewport()->update();
}

// Показываем модальный диалог настроек, затем переносим выбранные значения в состояние MainWindow
void MainWindow::openSettings()
{
    SettingsDialog dialog(this);

    dialog.setAutostartEnabled(isAutostartEnabled());
    dialog.setMinimizeToTray(minimizeToTray);
    dialog.setNotificationsEnabled(notificationsEnabled);
    dialog.setStartMinimized(startMinimized);
    dialog.setNotificationDuration(notificationDuration);
    dialog.setTheme(currentTheme());

    if (dialog.exec() != QDialog::Accepted) return;

    setAutostartEnabled(dialog.autostartEnabled());

    minimizeToTray = dialog.minimizeToTray();
    notificationsEnabled = dialog.notificationsEnabled();
    startMinimized = dialog.startMinimized();
    notificationDuration = dialog.notificationDuration();
    const QString theme = dialog.theme();

    QSettings settings;

    settings.setValue("appearance/theme", theme);
    saveSettings();

    applyTheme(theme);

    taskList->viewport()->update();

    if (autostartAction)
    {
        autostartAction->blockSignals(true);
        autostartAction->setChecked(isAutostartEnabled());
        autostartAction->blockSignals(false);
    }
}

// Таймер вызывает эту функцию каждую секунду, чтобы модель заново вычисляла OverdueRole и перерисовывала карточки
void MainWindow::updateOverdueTasks()
{
    if (tasks.isEmpty()) return;

    for ( int row = 0; row < tasks.size(); ++row)
    {
        taskModel->updateTask(row);
    }
    refreshTaskList();
}

// Создаём и настраиваем иконку системного трея и её контекстное меню
void MainWindow::setupTrayIcon()
{
    trayIcon->setIcon(QIcon(":/app.svg"));
    trayIcon->setToolTip("Task Manager");

    QAction *showAction = trayMenu->addAction("Открыть Task Manager");
    QAction *hideAction = trayMenu->addAction("Скрыть окно");

    trayMenu->addSeparator();

    QAction *settingsAction = trayMenu->addAction("Настройки");

    autostartAction = trayMenu->addAction("Запускать вместе с Windows");
    autostartAction->setCheckable(true);
    autostartAction->setChecked(isAutostartEnabled());

    trayMenu->addSeparator();

    QAction *quitAction = trayMenu->addAction("Выход");

    connect(showAction, &QAction::triggered, this, &MainWindow::showFromTray);
    connect(hideAction, &QAction::triggered, this, &MainWindow::hideToTray);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(autostartAction, &QAction::toggled, this, &MainWindow::setAutostartEnabled);
    connect(
        quitAction,
        &QAction::triggered,
        this,
        [this]()
        {
            reallyQuit = true;
            qApp->quit();
        }
    );

    trayMenu->setStyleSheet(qApp->styleSheet());
    trayIcon->setContextMenu(trayMenu);

    connect(
        trayIcon,
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason)
        {
            if (reason == QSystemTrayIcon::Trigger)
            {
                if (isVisible()) hideToTray();
                else showFromTray();
            }
        }
    );

    trayIcon->show();
}

bool MainWindow::isAutostartEnabled() const
{
#ifdef Q_OS_WIN

    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat
    );

    return settings.contains("TaskManager");

#else
    return false;
#endif
}

void MainWindow::setAutostartEnabled(
    bool enabled
)
{
#ifdef Q_OS_WIN

    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat
    );

    if (enabled)
    {
        const QString applicationPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue("TaskManager", "\"" + applicationPath + "\"");
    }
    else
    {
        settings.remove("TaskManager");
    }
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        if (autostartAction)
        {
            autostartAction->blockSignals(true);
            autostartAction->setChecked(!enabled);
            autostartAction->blockSignals(false);
        }
    }

#else

    Q_UNUSED(enabled);

#endif
}

void MainWindow::showFromTray()
{
    show();
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::hideToTray()
{
    hide();
}

void MainWindow::closeEvent(
    QCloseEvent *event
)
{
    if (reallyQuit)
    {
        event->accept();
        return;
    }

    if (minimizeToTray)
    {
        hide();

        event->ignore();

        trayIcon->showMessage("Task Manager", "Приложение продолжает работать в фоновом режиме.", QSystemTrayIcon::Information, 3000);

        return;
    }

    event->accept();
}

// Получаем сигнал от ReminderManager и показываем Windows-уведомление, если оно разрешено настройками
void MainWindow::showReminder(const Task &task)
{
    if (!notificationsEnabled) return;

    if (!trayIcon->isVisible()) return;

    trayIcon->showMessage("Задача просрочена", task.title, QSystemTrayIcon::Warning, notificationDuration);
}

// Обновляем сортировку и статистику. Само фильтрование выполняет proxyModel
void MainWindow::refreshTaskList()
{
    switch (sortMode->currentIndex())
    {
        case 1:
            proxyModel->setSortRole(TaskModel::DueDateRole);
            proxyModel->sort(0, Qt::AscendingOrder);
            break;
        case 2:
            proxyModel->setSortRole(TaskModel::PriorityRole);
            proxyModel->sort(0, Qt::DescendingOrder);
            break;

        default: break;
    }

    int completedCount = 0;

    for (const Task &task : tasks)
    {
        if (task.completed) ++completedCount;
    }

    statisticsLabel->setText(
        QString("Всего: %1 | " "Выполнено: %2 | " "Осталось: %3 | " "Показано: %4")
        .arg(tasks.size())
        .arg(completedCount)
        .arg(tasks.size() - completedCount)
        .arg(proxyModel->rowCount())
    );
}

// Создание: открыть TaskDialog - получить Task - присвоить ID - добавить - сохранить
void MainWindow::addTask()
{
    TaskDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) return;

    Task task = dialog.getTask();

    task.id = nextTaskId++;
    tasks.append(task);
    taskModel->setTasks(tasks);
    reminderManager->setTasks(tasks);
    saveTasks();
    refreshTaskList();
}

// Редактирование: переводим выбранный proxy-index в source-index, меняем Task и уведомляем модель
void MainWindow::editTask()
{
    const QModelIndex proxyIndex = taskList->currentIndex();

    if (!proxyIndex.isValid()) return;

    const QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

    Task *task = taskModel->getTask(sourceIndex.row());

    if (!task) return;

    TaskDialog dialog(this);

    dialog.setTask( *task);

    if (dialog.exec() != QDialog::Accepted) return;

    const int id = task->id;
    const bool completed = task->completed;

    *task = dialog.getTask();

    task->id = id;
    task->completed = completed;

    taskModel->updateTask(sourceIndex.row());

    tasks = taskModel->getTasks();

    reminderManager->setTasks(tasks);

    saveTasks();

    refreshTaskList();
}

// Удаление: переводим выбранную строку из proxy-модели в исходную и удаляем её из TaskModel
void MainWindow::deleteTask()
{
    const QModelIndex proxyIndex = taskList->currentIndex();
    if (!proxyIndex.isValid()) return;

    const QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    taskModel->removeTask(sourceIndex.row());
    tasks = taskModel->getTasks();
    reminderManager->setTasks(tasks);

    saveTasks();
    refreshTaskList();
}

// Переключаем completed. После изменения модель получает dataChanged(), а данные сохраняются в JSON
void MainWindow::toggleTaskCompleted()
{
    const QModelIndex proxyIndex = taskList->currentIndex();
    if (!proxyIndex.isValid()) return;

    const QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    Task *task = taskModel->getTask(sourceIndex.row());
    if (!task) return;

    task->completed = !task->completed;
    taskModel->updateTask(sourceIndex.row());
    tasks = taskModel->getTasks();
    reminderManager->setTasks(tasks);

    saveTasks();
    refreshTaskList();
}

Task *MainWindow::findTaskById(int id)
{
    for (Task &task : tasks)
    {
        if (task.id == id) return &task;
    }

    return nullptr;
}

// Возвращаем поиск, фильтр и сортировку к начальному состоянию
void MainWindow::resetFilters()
{
    searchInput->clear();
    statusFilter->setCurrentIndex(0);
    sortMode->setCurrentIndex(0);

    proxyModel->setSearchText("");
    proxyModel->setStatusFilter(TaskFilterProxyModel::StatusFilter::All);

    taskList->clearSelection();

    refreshTaskList();
}

// Передаём текущий список задач в TaskStorage для записи в tasks.json
void MainWindow::saveTasks()
{
    storage.save(tasks);
}