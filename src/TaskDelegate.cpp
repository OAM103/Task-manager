/* TaskDelegate.cpp - ручная отрисовка карточки задачи
 *
 * Делегат НЕ изменяет задачи и НЕ хранит их. Он получает QModelIndex, запрашивает у TaskModel нужные роли и рисует результат через QPainter.
 *
 * Поэтому цвета, зачёркивание выполненной задачи, красный статус просрочки и цветная полоска слева — это только визуальный слой.
 */
#include "TaskDelegate.hpp"
#include "TaskModel.hpp"
#include <QPainter>
#include <QPalette>
#include <QStyle>

TaskDelegate::TaskDelegate(QObject *parent) : QStyledItemDelegate(parent){}

// Рисуем одну карточку задачи. Все сведения берём через роли TaskModel
void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
painter->save();

const QRect rect = option.rect.adjusted(6, 4, -6, -4);

const bool selected = option.state & QStyle::State_Selected;

const bool completed = index.data(TaskModel::CompletedRole).toBool();

const bool overdue = index.data(TaskModel::OverdueRole).toBool();

const int priority = index.data(TaskModel::PriorityRole).toInt();

const QPalette palette = option.palette;

// Определяем текущую тему по цвету фона палитры
const QColor windowColor = palette.color(QPalette::Window);

const bool darkTheme = windowColor.lightness() < 128;

// Цвет обычной карточки
const QColor cardColor = darkTheme ? QColor("#252a33") : QColor("#ffffff");

// Цвет выделенной карточки
const QColor selectedColor = darkTheme ? QColor("#344563") : QColor("#dfe8ff");

painter->setBrush(selected ? selectedColor : cardColor);
painter->setPen(Qt::NoPen);
painter->drawRoundedRect( rect, 12, 12);

/* Цвет полоски слева.
 *
 * Просроченная задача имеет приоритет над обычным цветом приоритета

 * просрочена - красная
 * выполнена  - серая
 * иначе      - цвет приоритета
 */
QColor priorityColor;

if (completed) priorityColor =  darkTheme ? QColor("#64748b") : QColor("#94a3b8");
else if (overdue)  priorityColor = QColor("#ef4444");
else
{
    switch (priority)
    {
        case static_cast<int>(TaskPriority::Low):
            priorityColor = QColor("#22c55e");
            break;

        case static_cast<int>(TaskPriority::High):
            priorityColor = QColor("#ef4444");
            break;

        default:
            priorityColor = QColor("#6366f1");
            break;
    }
}

QRect priorityRect(rect.left() + 12, rect.top() + 12, 5, rect.height() - 24);

painter->setBrush(priorityColor);
painter->drawRoundedRect(priorityRect, 2, 2);

const QString displayText = index.data(Qt::DisplayRole).toString();
const QString title =
    displayText
        .section('\n', 0, 0)
        .mid(3);

const QString details =
    displayText
        .section('\n', 1, 1);

const QString description =
    displayText
        .section('\n', 2);

const int contentLeft =
    rect.left() + 30;

const int contentWidth =
    rect.width() - 42;

// Цвет основного текста
const QColor textColor = darkTheme ? QColor("#f1f5f9") : QColor("#20242c");

// Цвет второстепенного текста
const QColor secondaryTextColor = darkTheme ? QColor("#aeb8c7") : QColor("#6b7280");

// Цвет завершённой задачи
const QColor completedTextColor = darkTheme ? QColor("#737d8c") : QColor("#9ca3af");

// Название задачи
QFont titleFont = option.font;

titleFont.setBold(true);
titleFont.setPointSize(titleFont.pointSize() + 1);

if (completed) titleFont.setStrikeOut(true);

painter->setFont(titleFont);
painter->setPen(completed ? completedTextColor : textColor);

QRect titleRect(contentLeft, rect.top() + 10, contentWidth, 25);

painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

// Срок / статус задачи
QFont detailsFont = option.font;

detailsFont.setPointSize(qMax(8, detailsFont.pointSize() - 1));

painter->setFont(detailsFont);

// Для просроченной задачи делаем статус также красным
if (overdue && !completed) painter->setPen(QColor("#ef4444"));
else painter->setPen(secondaryTextColor);

QRect detailsRect(contentLeft, rect.top() + 37, contentWidth, 22);

painter->drawText(detailsRect, Qt::AlignLeft | Qt::AlignVCenter, details);

// Описание задачи
if (!description.isEmpty())
{
    painter->setPen(secondaryTextColor);
    QRect descriptionRect(contentLeft, rect.top() + 61, contentWidth, 40);
    painter->drawText(descriptionRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, description);
}

painter->restore();

}

//Сообщаем QListView, сколько вертикального места нужно одной карточке
QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
Q_UNUSED(option);

const QString displayText = index.data(Qt::DisplayRole).toString();
const QString description = displayText.section('\n', 2);
if (description.isEmpty())
{
    return QSize(0, 86);
}
return QSize(0, 126);

}
