#pragma once
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>

class MessageDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit MessageDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setSearchKeyword(const QString& kw);
private:
    QString m_keyword;
};
