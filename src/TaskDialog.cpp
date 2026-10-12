/* TaskDialog.cpp - форма создания и редактирования задачи.
 *
 * Здесь находятся элементы ввода, быстрый выбор срока и проверка данных перед нажатием «Сохранить». Метод getTask() превращает содержимое формы обратно в обычную структуру Task
 */
#include "TaskDialog.hpp"

#include <QCalendarWidget>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QVBoxLayout>

TaskDialog::TaskDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Новая задача");
    setMinimumWidth(500);

    titleInput = new QLineEdit(this);
    titleInput->setPlaceholderText("Название задачи");

    descriptionInput = new QTextEdit(this);
    descriptionInput->setPlaceholderText("Описание задачи");
    descriptionInput->setMinimumHeight(110);

    priorityInput =new QComboBox(this);
    priorityInput->addItem("Низкий");
    priorityInput->addItem("Обычный");
    priorityInput->addItem("Высокий");

    quickDeadlineInput = new QComboBox(this);
    quickDeadlineInput->addItem("Выбрать срок...");

    quickDeadlineInput->addItem("Через 15 минут");
    quickDeadlineInput->addItem("Через 30 минут");
    quickDeadlineInput->addItem("Через 1 час");
    quickDeadlineInput->addItem("Через 2 часа");
    quickDeadlineInput->addItem("Через день");
    quickDeadlineInput->addItem("Завтра в это же время");
    quickDeadlineInput->addItem("Через неделю");

    dueDateTimeInput = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600), this);

    dueDateTimeInput->setCalendarPopup(true);
    dueDateTimeInput->setDisplayFormat("dd.MM.yyyy HH:mm");
    dueDateTimeInput->setMinimumWidth(190);

    // Запрещаем выбирать дату в прошлом
    dueDateTimeInput->setMinimumDate(QDate::currentDate());
    dueDateTimeInput->setMinimumDateTime(QDateTime::currentDateTime());

    // Настраиваем визуальное отображение прошедших дат
    updateCalendarAppearance();

    auto *formLayout = new QFormLayout;

    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->addRow("Название:", titleInput);
    formLayout->addRow("Описание:", descriptionInput);
    formLayout->addRow("Приоритет:", priorityInput);
    formLayout->addRow("Быстрый срок:", quickDeadlineInput);
    formLayout->addRow("Точный срок:", dueDateTimeInput);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    buttons->button(QDialogButtonBox::Ok)->setText("Сохранить");
    buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");

    auto *layout = new QVBoxLayout(this);

    layout->addLayout(formLayout);
    layout->addWidget(buttons);

    connect(
        quickDeadlineInput,
        &QComboBox::currentIndexChanged,
        this,
        [this](int index)
        {
            applyQuickDeadline(index);
        }
    );

    connect(
        buttons,
        &QDialogButtonBox::accepted,
        this,
        [this]()
        {
            if (titleInput->text().trimmed().isEmpty())
            {
                titleInput->setFocus();
                return;
            }

            const QDateTime now = QDateTime::currentDateTime();

            if (!dueDateTimeInput->dateTime().isValid()||dueDateTimeInput->dateTime() < now)
            {
                dueDateTimeInput->setMinimumDateTime(now);
                dueDateTimeInput->setDateTime(now.addSecs(3600));

                updateCalendarAppearance();
                dueDateTimeInput->setFocus();

                return;
            }

            accept();
        }
    );

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Форматируем календарь: прошедшие даты делаем серыми для визуальной подсказки
void TaskDialog::updateCalendarAppearance()
{
    if (!dueDateTimeInput) return;

    QCalendarWidget *calendar = dueDateTimeInput->calendarWidget();

    if (!calendar)return;

    /* Серый цвет для дат, которые уже прошли
     *
     * Мы используем нейтральный серый, поэтому он хорошо читается и в светлой, и в тёмной теме
     */
    QTextCharFormat pastFormat;

    pastFormat.setForeground(QColor(150, 150, 150));

    const QDate today = QDate::currentDate();

    // Очищаем старые форматы
    calendar->setDateTextFormat(QDate(), QTextCharFormat());

    /* Проходим по небольшому диапазону месяцев вокруг текущего отображаемого месяца
     *
     * Этого достаточно для нормальной навигации пользователя по календарю
     */
    const QDate firstDay = QDate(today.year(), today.month(), 1);
    const QDate startDate = firstDay.addMonths(-24);
    const QDate endDate = firstDay.addMonths(24);
    for (QDate date = startDate; date <= endDate; date = date.addDays(1))
    {
        if (date < today) calendar->setDateTextFormat(date, pastFormat);
    }
}

void TaskDialog::updateCalendarForMonth(int year, int month)
{
    Q_UNUSED(year);
    Q_UNUSED(month);

    updateCalendarAppearance();
}

// Переводим выбранный пункт «через N...» в конкретный QDateTime
void TaskDialog::applyQuickDeadline(int index)
{
    if (index == 0) return;

    const QDateTime now = QDateTime::currentDateTime();

    QDateTime deadline;

    switch (index)
    {
        case 1:
            deadline = now.addSecs(15 * 60);
            break;
        case 2:
            deadline = now.addSecs(30 * 60);
            break;
        case 3:
            deadline = now.addSecs(60 * 60);
            break;
        case 4:
            deadline = now.addSecs(2 * 60 * 60);
            break;
        case 5:
            deadline = now.addDays(1);
            break;
        case 6:
            deadline = now.addDays(1);
            break;
        case 7:
            deadline = now.addDays(7);
            break;
        default:
            return;
    }

    dueDateTimeInput->setMinimumDate(QDate::currentDate());
    dueDateTimeInput->setMinimumDateTime(now);
    dueDateTimeInput->setDateTime(deadline);
    updateCalendarAppearance();
}

// Заполняем форму существующей задачей перед редактированием
void TaskDialog::setTask(const Task &task)
{
    titleInput->setText(task.title);

    descriptionInput->setPlainText(task.description);

    switch (task.priority)
    {
        case TaskPriority::Low: 
            priorityInput->setCurrentIndex(0);
            break;
        case TaskPriority::Normal:
            priorityInput->setCurrentIndex(1);
            break;
        case TaskPriority::High:
            priorityInput->setCurrentIndex(2);
            break;
    }

    const QDateTime now = QDateTime::currentDateTime();

    dueDateTimeInput->setMinimumDate(QDate::currentDate());
    dueDateTimeInput->setMinimumDateTime(now);

    if (task.dueDateTime.isValid() && task.dueDateTime >= now) dueDateTimeInput->setDateTime(task.dueDateTime);
    else dueDateTimeInput->setDateTime(now.addSecs(3600));

    quickDeadlineInput->setCurrentIndex(0);

    updateCalendarAppearance();

    setWindowTitle("Редактирование задачи");
}

// Собираем введённые пользователем значения обратно в структуру Task
Task TaskDialog::getTask() const
{
    Task task;

    task.title = titleInput->text().trimmed();

    task.description = descriptionInput->toPlainText().trimmed();

    switch (priorityInput->currentIndex())
    {
        case 0:
            task.priority = TaskPriority::Low;
            break;
        case 1:
            task.priority = TaskPriority::Normal;
            break;

        case 2:
            task.priority = TaskPriority::High;
            break;
    }

    task.dueDateTime = dueDateTimeInput->dateTime();

    return task;
}