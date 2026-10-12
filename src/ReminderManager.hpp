/*
 * ReminderManager.hpp - таймер проверки просроченных задач
 *
 * Класс получает копию списка задач, каждую секунду проверяет сроки и один раз для каждой задачи испускает reminderTriggered(). MainWindow принимает этот сигнал и показывает уведомление в системном трее
 */
#pragma once

#include <QObject>
#include <QSet>
#include <QTimer>
#include <QList>

#include "Task.hpp"

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);

    void setTasks(const QList<Task> &tasks);

signals:
    void reminderTriggered(const Task &task);

private slots:
    void checkTasks();

private:
    QList<Task> tasks;
    QSet<int> notifiedTaskIds;
    QTimer timer;
};