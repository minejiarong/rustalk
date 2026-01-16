#include "MessageDelegate.h"
#include "MessageListModel.h"
#include <QFontMetrics>
#include <QPalette>
#include <QTextLayout>
#include <QTextOption>

MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void MessageDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    p->save();
    QRect r = opt.rect;
    QString text = idx.data(MessageListModel::ContentRole).toString();
    bool isOwn = idx.data(MessageListModel::IsOwnRole).toBool();
    bool isDivider = idx.data(MessageListModel::IsDividerRole).toBool();
    if (isDivider) {
        // Draw date divider centered
        p->setRenderHint(QPainter::Antialiasing, true);
        QColor bar = QColor("#4A4A4A");
        QColor labelBg = QColor("#2E2E2E");
        QColor labelFg = QColor("#CCCCCC");
        int barY = r.center().y();
        int margin = 20;
        // left and right line
        p->setPen(QPen(bar, 1));
        int labelWidth = qMin(r.width() * 0.4, 220.0);
        int labelHeight = 24;
        int labelX = r.center().x() - labelWidth / 2;
        // left line
        p->drawLine(r.left() + margin, barY, labelX - 8, barY);
        // right line
        p->drawLine(labelX + labelWidth + 8, barY, r.right() - margin, barY);
        // label
        QRect labelRect(labelX, barY - labelHeight / 2, labelWidth, labelHeight);
        p->setBrush(labelBg);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(labelRect, 6, 6);
        p->setPen(labelFg);
        p->drawText(labelRect, Qt::AlignCenter, text);
        p->restore();
        return;
    }
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
    if (!m_keyword.isEmpty()) {
        QTextLayout layout(text, opt.font);
        QTextOption to;
        to.setWrapMode(QTextOption::WordWrap);
        layout.setTextOption(to);
        QVector<QTextLayout::FormatRange> ranges;
        int start = 0;
        while (true) {
            int pos = text.indexOf(m_keyword, start, Qt::CaseInsensitive);
            if (pos < 0) break;
            QTextLayout::FormatRange fr;
            fr.start = pos;
            fr.length = m_keyword.length();
            QTextCharFormat fmt;
            fmt.setBackground(QColor("#FFD54F"));
            fmt.setForeground(fg);
            fr.format = fmt;
            ranges.push_back(fr);
            start = pos + fr.length;
        }
        layout.setAdditionalFormats(ranges);
        layout.beginLayout();
        qreal h = 0;
        while (true) {
            QTextLine line = layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(inner.width());
            line.setPosition(QPointF(0, h));
            h += line.height();
        }
        layout.endLayout();
        layout.draw(p, inner.topLeft());
    } else {
        p->drawText(inner, Qt::TextWordWrap, text);
    }
    p->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    QString text = idx.data(MessageListModel::ContentRole).toString();
    bool isDivider = idx.data(MessageListModel::IsDividerRole).toBool();
    if (isDivider) {
        return QSize(opt.rect.width(), 36);
    }
    QFontMetrics fm(opt.font);
    int maxWidth = opt.rect.width() * 0.7;
    QRect textRect = fm.boundingRect(0, 0, maxWidth, 0, Qt::TextWordWrap, text);
    int padding = 10;
    return QSize(opt.rect.width(), textRect.height() + padding * 2 + 10);
}

void MessageDelegate::setSearchKeyword(const QString& kw) {
    m_keyword = kw.trimmed();
}
