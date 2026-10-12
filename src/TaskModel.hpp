/* TaskModel.hpp - модель данных Qt для списка задач
 *
 * QListView не работает напрямую с QList<Task>. Qt ожидает специальный объект
 * модели, который умеет отвечать на вопросы: сколько строк есть, какие данные
 * лежат в строке и когда данные изменились. Именно это делает TaskModel.
 *
 * Roles — дополнительные «каналы» данных для сортировки, фильтрации и отрисовки.
 */
#pragma once

#include <QAbstractListModel>
#include <QList>

#include "Task.hpp"

class TaskModel : public QAbstractListModel
{
public:

    // Роли Qt - это дополнительные имена для разных представлений одной задачи
    // QListView использует DisplayRole для текста, а сортировка/Delegate могут отдельно запросить CompletedRole, PriorityRole, DueDateRole и OverdueRole
    enum Roles
    {
        TaskIdRole = Qt::UserRole + 1,
        CompletedRole,
        PriorityRole,
        DueDateRole,
        OverdueRole
    };

    explicit TaskModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setTasks(const QList<Task> &newTasks);

    const QList<Task> &getTasks() const;

    Task *getTask(int row);

    void updateTask(int row);

    void addTask(const Task &task);

    void removeTask(int row);

private: QList<Task> tasks;
};