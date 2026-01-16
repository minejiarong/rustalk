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
    const int pos = m_items.size();
    beginInsertRows(QModelIndex(), pos, pos);
    m_items.push_back({from, to, content, timestamp});
    endInsertRows();
}

void MessageListModel::loadHistory(int limit) {
    clear();
    int out_len = 0;
    MessageFFI* msgs = rustalk_fetch_history(m_peerId, limit, &out_len);
    if (!msgs || out_len <= 0) return;
    for (int i = 0; i < out_len; ++i) {
        const auto &m = msgs[i];
        QString text = QString::fromUtf8(m.content);
        appendMessage(m.from, m.to, text, m.timestamp);
    }
    rustalk_free_messages(msgs, out_len);
}
