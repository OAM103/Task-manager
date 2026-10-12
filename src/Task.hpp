/*
 * Task.hpp — базовая структура данных одной задачи: идентификатор, название, описание, приоритет, срок и признак выполнения
 */
#pragma once

#include <QDateTime>
#include <QString>


// Приоритет хранится как enum class, чтобы значения Low/Normal/High были отдельным типом и случайно не смешивались с обычными int
enum class TaskPriority
{
    Low,
    Normal,
    High
};

struct Task
{
    int id = 0;

    QString title;
    QString description;

    TaskPriority priority =
        TaskPriority::Normal;

    QDateTime dueDateTime;

    bool completed = false;
};