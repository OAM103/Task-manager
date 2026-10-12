/*
 * ReminderManager.cpp - реализация проверки сроков
 *
 * QTimer вызывает checkTasks() каждую секунду. QSet notifiedTaskIds нужен для защиты от повторных уведомлений по одной и той же задаче
 */
#include "ReminderManager.hpp"

#include <QDateTime>

ReminderManager::ReminderManager(QObject *parent): QObject(parent)
{
    timer.setInterval(1000);

    connect(&timer, &QTimer::timeout, this, &ReminderManager::checkTasks);

    timer.start();
}

// Обновляем список задач для проверки и очищаем IDs уведомлений удалённых задач
void ReminderManager::setTasks(const QList<Task> &newTasks)
{
    tasks = newTasks;

    QSet<int> existingIds;

    for (const Task &task : tasks)
    {
        existingIds.insert(task.id);
    }

    QSet<int> cleanedNotifications;

    for (const int id :notifiedTaskIds)
    {
        if (existingIds.contains(id))cleanedNotifications.insert(id);
    }

    notifiedTaskIds = cleanedNotifications;

    checkTasks();
}

// Проверяем сроки. Каждое просроченное активное задание уведомляет только один раз
void ReminderManager::checkTasks()
{
    const QDateTime now = QDateTime::currentDateTime();

    for (const Task &task : tasks)
    {
        if (task.completed) continue;

        if (!task.dueDateTime.isValid()) continue;

        if (task.dueDateTime > now) continue;

        if (notifiedTaskIds.contains(task.id)) continue;

        notifiedTaskIds.insert(task.id);

        emit reminderTriggered(task);
    }
}