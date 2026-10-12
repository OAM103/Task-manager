/*TaskModel.cpp - реализация модели списка задач
 *
 * Главный принцип Qt Model/View: TaskModel хранит данные - QListView запрашивает data() - TaskDelegate рисует полученные данные
 *
 * dataChanged(), beginInsertRows() и beginRemoveRows() нужны для того, чтобы представление узнало об изменениях и перерисовалось правильно
 */
#include "TaskModel.hpp"
#include <QDateTime>

TaskModel::TaskModel(QObject *parent): QAbstractListModel(parent){}

// Qt спрашивает количество строк, которые нужно показать в QListView
int TaskModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return tasks.size();
}

// Главная функция модели: по role возвращает нужное представление одной задачи
QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= tasks.size()) return {};

    const Task &task =tasks[index.row()];

    const bool overdue = !task.completed && task.dueDateTime.isValid() && task.dueDateTime < QDateTime::currentDateTime();

    switch (role)
    {
        case Qt::DisplayRole:
        {
            QString status;

            if (task.completed) status = "✓";
            else if (overdue) status = "!";
            else status = "○";

            QString priority;

            switch (task.priority)
            {
                case TaskPriority::Low:
                    priority = "низкий";
                    break;

                case TaskPriority::Normal:
                    priority = "обычный";
                    break;

                case TaskPriority::High:
                    priority = "высокий";
                    break;
            }

            QString deadlineStatus;

            if (task.completed) deadlineStatus = "Выполнена";
            else if (overdue) deadlineStatus = "ПРОСРОЧЕНА";
            else deadlineStatus ="Активна";

            return QString("%1  %2\n" "%3\n" "%4 | %5 | %6\n" "%7")
            .arg(status)
            .arg(task.title)
            .arg(task.dueDateTime.toString("dd.MM.yyyy HH:mm"))
            .arg(deadlineStatus)
            .arg(priority)
            .arg(task.description)
            .arg(task.completed ? "Задача завершена" : "Задача в работе");
        }

        case TaskIdRole:
            return task.id;

        case CompletedRole:
            return task.completed;

        case PriorityRole:
            return static_cast<int>(task.priority);

        case DueDateRole:
            return task.dueDateTime;

        case OverdueRole:
            return overdue;

        default:
            return {};
    }
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// Полностью заменяем список. begin/endResetModel сообщают View, что модель перестроена
void TaskModel::setTasks(const QList<Task> &newTasks)
{
    beginResetModel();
    tasks = newTasks;
    endResetModel();
}

const QList<Task> &
TaskModel::getTasks() const
{
    return tasks;
}

Task *TaskModel::getTask(
    int row
)
{
    if (row < 0 || row >= tasks.size())
    {
        return nullptr;
    }

    return &tasks[row];
}

// Сообщаем Qt, что одна строка изменилась; View и Delegate должны запросить data() заново
void TaskModel::updateTask(int row)
{
    if (row < 0 || row >= tasks.size()) return;
    emit dataChanged(index(row),index(row));
}

// Добавление строки по правилам Qt Model/View: сначала beginInsertRows(), потом изменение контейнера
void TaskModel::addTask(const Task &task)
{
    const int row = tasks.size();

    beginInsertRows(QModelIndex(), row, row);
    tasks.append(task);
    endInsertRows();
}

// Удаление строки по правилам Qt Model/View
void TaskModel::removeTask(int row)
{
    if (row < 0 || row >= tasks.size())return;
    
    beginRemoveRows(QModelIndex(), row, row);

    tasks.removeAt(row);

    endRemoveRows();
}