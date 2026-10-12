/* TaskFilterProxyModel.hpp - промежуточная модель между TaskModel и QListView
 *
 * Она не хранит второй список задач. Она смотрит на TaskModel и решает, какие строки показывать пользователю и в каком порядке их сортировать
 */
#pragma once
#include <QSortFilterProxyModel>

class TaskFilterProxyModel : public QSortFilterProxyModel
{
public:
    enum class StatusFilter
    {
        All,
        Active,
        Completed
    };

    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    void setSearchText(const QString &text);

    void setStatusFilter(StatusFilter filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString searchText;
    StatusFilter statusFilter = StatusFilter::All;
};