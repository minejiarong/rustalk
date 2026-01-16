#include "MessageListModel.h"
#include <QDateTime>

MessageListModel::MessageListModel(QObject *parent) : QAbstractListModel(parent) {}

int MessageListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant MessageListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    const auto &it = m_items.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
        case ContentRole:
            return it.content;
        case FromRole:
            return it.from;
        case ToRole:
            return it.to;
        case TimestampRole:
            return it.timestamp;
        case IsOwnRole:
            return it.from == m_currentUserId;
        case SenderRole:
            return it.from;
        case IsDividerRole:
            return it.isDivider;
        default:
            return {};
    }
}

QHash<int, QByteArray> MessageListModel::roleNames() const {
    QHash<int, QByteArray> r;
    r[ContentRole] = "content";
    r[FromRole] = "from";
    r[ToRole] = "to";
    r[TimestampRole] = "timestamp";
    r[IsOwnRole] = "isOwn";
    r[SenderRole] = "sender";
    r[IsDividerRole] = "isDivider";
    return r;
}

void MessageListModel::setCurrentUserId(qint64 uid) {
    m_currentUserId = uid;
}

void MessageListModel::setCurrentPeer(qint64 peerId, bool isGroup) {
    m_peerId = peerId;
    m_isGroup = isGroup;
}

void MessageListModel::clear() {
    beginResetModel();
    m_items.clear();
    endResetModel();
}

void MessageListModel::appendMessage(qint64 from, qint64 to, const QString &content, qint64 timestamp) {
    // Check if need to insert a date divider before the new message (when appending at end)
    QDate newDate = QDateTime::fromMSecsSinceEpoch(timestamp).date();
    QDate lastDate;
    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (!m_items[i].isDivider) {
            lastDate = QDateTime::fromMSecsSinceEpoch(m_items[i].timestamp).date();
            break;
        }
    }
    int insertCount = 1;
    if (!lastDate.isValid() || lastDate != newDate) {
        insertCount = 2;
    }
    const int startRow = m_items.size();
    beginInsertRows(QModelIndex(), startRow, startRow + insertCount - 1);
    if (insertCount == 2) {
        QString dateText = newDate.toString("yyyy-MM-dd");
        m_items.push_back({0, 0, 0, dateText, timestamp, true});
    }
    m_items.push_back({0, from, to, content, timestamp, false});
    endInsertRows();
}

void MessageListModel::loadHistory(int limit) {
    clear();
    int out_len = 0;
    MessageFFI* msgs = rustalk_fetch_history(m_peerId, limit, &out_len);
    if (!msgs || out_len <= 0) return;
    QDate prevDate;
    for (int i = 0; i < out_len; ++i) {
        const auto &m = msgs[i];
        QString text = QString::fromUtf8(m.content);
        QDate curDate = QDateTime::fromMSecsSinceEpoch(m.timestamp).date();
        if (!prevDate.isValid() || curDate != prevDate) {
            QString dateText = curDate.toString("yyyy-MM-dd");
            m_items.push_back({0, 0, 0, dateText, m.timestamp, true});
            prevDate = curDate;
        }
        m_items.push_back({m.id, m.from, m.to, text, m.timestamp, false});
    }
    rustalk_free_messages(msgs, out_len);
    if (!m_items.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, m_items.size() - 1);
        endInsertRows();
    }
}

void MessageListModel::searchMessages(const QString &keyword) {
    if (keyword.isEmpty()) {
        loadHistory(50);
        return;
    }
    
    clear();
    int out_len = 0;
    MessageFFI* msgs = rustalk_search_history(m_peerId, keyword.toUtf8().constData(), &out_len);
    if (!msgs || out_len <= 0) return;
    
    QDate prevDate;
    for (int i = 0; i < out_len; ++i) {
        const auto &m = msgs[i];
        QString text = QString::fromUtf8(m.content);
        QDate curDate = QDateTime::fromMSecsSinceEpoch(m.timestamp).date();
        if (!prevDate.isValid() || curDate != prevDate) {
            QString dateText = curDate.toString("yyyy-MM-dd");
            m_items.push_back({0, 0, 0, dateText, m.timestamp, true});
            prevDate = curDate;
        }
        m_items.push_back({m.id, m.from, m.to, text, m.timestamp, false});
    }
    rustalk_free_messages(msgs, out_len);
    
    if (!m_items.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, m_items.size() - 1);
        endInsertRows();
    }
}

void MessageListModel::deleteMessage(int row) {
    if (row < 0 || row >= m_items.size()) return;
    const auto &item = m_items.at(row);
    if (item.isDivider) {
        return;
    }
    if (item.id > 0) {
        rustalk_delete_message(item.id);
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}
