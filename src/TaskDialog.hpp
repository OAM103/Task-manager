/*
 * TaskDialog.hpp - объявление диалога создания/редактирования задачи
 * Диалог отвечает только за ввод данных. Сохранение задачи на диск выполняет MainWindow через TaskStorage
 */
#pragma once

#include <QDialog>

class QComboBox;
class QDate;
class QDateTime;
class QDateTimeEdit;
class QLineEdit;
class QTextEdit;

#include "Task.hpp"

class TaskDialog : public QDialog
{
public:
    explicit TaskDialog(QWidget *parent = nullptr);

    void setTask(const Task &task);

    Task getTask() const;

private:
    void applyQuickDeadline(int index);

    void updateCalendarAppearance();

    void updateCalendarForMonth(int year, int month);

    QLineEdit *titleInput;
    QTextEdit *descriptionInput;
    QComboBox *priorityInput;
    QComboBox *quickDeadlineInput;
    QDateTimeEdit *dueDateTimeInput;
};