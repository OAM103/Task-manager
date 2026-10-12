/* TaskDelegate.hpp - объявление собственного делегата рисования задачи
 *
 * Стандартный QListView мог бы вывести обычный текст. Здесь мы переопределяем paint(), поэтому каждая задача отображается как собственная карточка.
 */
#pragma once

#include <QStyledItemDelegate>

class TaskDelegate : public QStyledItemDelegate
{
public:
    explicit TaskDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
