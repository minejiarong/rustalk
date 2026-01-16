#include "MessageDelegate.h"
#include "MessageListModel.h"
#include <QFontMetrics>
#include <QPalette>

MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void MessageDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    p->save();
    QRect r = opt.rect;
    QString text = idx.data(MessageListModel::ContentRole).toString();
    bool isOwn = idx.data(MessageListModel::IsOwnRole).toBool();
    QFontMetrics fm(opt.font);
    int maxWidth = r.width() * 0.7;
    QRect textRect = fm.boundingRect(0, 0, maxWidth, 0, Qt::TextWordWrap, text);
    int padding = 10;
    QSize bubbleSize(textRect.width() + padding * 2, textRect.height() + padding * 2);
    int x;
    if (isOwn) {
        x = r.right() - bubbleSize.width() - 10;
    } else {
        x = r.left() + 10;
    }
    QRect bubbleRect(QPoint(x, r.top() + 5), bubbleSize);
    QColor bg = isOwn ? QColor("#007ACC") : QColor("#333333");
    QColor fg = QColor("#FFFFFF");
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBrush(bg);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(bubbleRect, 8, 8);
    p->setPen(fg);
    QRect inner = bubbleRect.adjusted(padding, padding, -padding, -padding);
    p->drawText(inner, Qt::TextWordWrap, text);
    p->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    QString text = idx.data(MessageListModel::ContentRole).toString();
    QFontMetrics fm(opt.font);
    int maxWidth = opt.rect.width() * 0.7;
    QRect textRect = fm.boundingRect(0, 0, maxWidth, 0, Qt::TextWordWrap, text);
    int padding = 10;
    return QSize(opt.rect.width(), textRect.height() + padding * 2 + 10);
}
