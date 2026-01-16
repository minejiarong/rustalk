#pragma once
#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QVariant>
#include "rustalk_core.h"

class MessageListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        ContentRole = Qt::UserRole + 1,
        FromRole,
        ToRole,
        TimestampRole,
        IsOwnRole,
        SenderRole
    };

    explicit MessageListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setCurrentUserId(qint64 uid);
    void setCurrentPeer(qint64 peerId, bool isGroup);
    void clear();
    void appendMessage(qint64 from, qint64 to, const QString &content, qint64 timestamp);
    void loadHistory(int limit);

private:
    struct Item {
        qint64 from;
        qint64 to;
        QString content;
        qint64 timestamp;
    };
    QVector<Item> m_items;
    qint64 m_currentUserId{0};
    qint64 m_peerId{0};
    bool m_isGroup{false};
};
