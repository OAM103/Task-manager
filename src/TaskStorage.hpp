/*
 * TaskStorage.hpp - слой сохранения задач
 *
 * Этот класс знает только о файле tasks.json и формате JSON. Он не знает ничего о кнопках, QListView или системном трее. Это отделяет хранение данных от интерфейса.
 */
#pragma once

#include <QList>
#include <QString>

#include "Task.hpp"

class TaskStorage
{
public:
    explicit TaskStorage(const QString &filePath);

    bool save(const QList<Task> &tasks) const;
    bool load(QList<Task> &tasks) const;

private:
    QString filePath;
};