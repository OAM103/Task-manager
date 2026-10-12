/* TaskFilterProxyModel.cpp - поиск и фильтрация задач
 *
 * Пользовательский текст и выбранный статус здесь превращаются в условие filterAcceptsRow(). Если строка не подходит, она скрывается в QListView.
 */
#include "TaskFilterProxyModel.hpp"
#include "TaskModel.hpp"

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void TaskFilterProxyModel::setSearchText(const QString &text)
{
    searchText = text.trimmed();
    invalidateFilter();
}

void TaskFilterProxyModel::setStatusFilter(StatusFilter filter)
{
    statusFilter = filter;
    invalidateFilter();
}

// Для каждой строки исходной модели решаем: показывать её пользователю или скрыть
bool TaskFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString title = index.data(Qt::DisplayRole).toString();
    const bool completed = index.data(TaskModel::CompletedRole).toBool();

    if (statusFilter == StatusFilter::Active && completed) return false;
    if (statusFilter == StatusFilter::Completed && !completed) return false;
    if (searchText.isEmpty()) return true;

    return title.contains(searchText, Qt::CaseInsensitive);
}