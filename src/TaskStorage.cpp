/*
 * TaskStorage.cpp - сериализация Task - JSON
 *
 * save(): QList<Task> -> QJsonArray -> файл tasks.json
 * load(): файл tasks.json -> QJsonDocument -> QList<Task>
 *
 * ISODate используется для даты/времени, чтобы значение можно было корректно восстановить после перезапуска приложения
 */
#include "TaskStorage.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

TaskStorage::TaskStorage(const QString &filePath): filePath(filePath){}

/* Преобразуем каждую Task в QJsonObject и записываем массив JSON в файл. */
bool TaskStorage::save(const QList<Task> &tasks) const
{
    QJsonArray array;

    for (const Task &task : tasks)
    {
        QJsonObject object;

        object["id"] = task.id;
        object["title"] = task.title;
        object["description"] = task.description;
        object["priority"] = static_cast<int>(task.priority);
        object["dueDateTime"] = task.dueDateTime.toString(Qt::ISODate);
        object["completed"] = task.completed;

        array.append(object);
    }

    QJsonDocument document(array);

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(document.toJson(QJsonDocument::Indented));

    return true;
}

// Читаем JSON, проверяем структуру и восстанавливаем QList<Task>
bool TaskStorage::load(QList<Task> &tasks) const
{
    QFile file(filePath);

    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QByteArray data = file.readAll();

    QJsonParseError error;

    QJsonDocument document = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) return false;
    if (!document.isArray()) return false;

    tasks.clear();

    const QJsonArray array = document.array();

    for (const QJsonValue &value : array)
    {
        if (!value.isObject()) continue;

        const QJsonObject object = value.toObject();

        Task task;

        task.id = object["id"].toInt();
        task.title = object["title"].toString();
        task.description = object["description"].toString();

        const int priority = object["priority"].toInt(1);

        if (priority >= 0 &&  priority <= 2) task.priority = static_cast<TaskPriority>(priority );
        else task.priority = TaskPriority::Normal;

        task.dueDateTime = QDateTime::fromString(object["dueDateTime"].toString(),Qt::ISODate);
        task.completed = object["completed"].toBool(false);
        
        tasks.append(task);
    }

    return true;
}