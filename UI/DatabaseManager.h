#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Read,
        Write
    };

    explicit DatabaseManager(QObject *parent = nullptr);
    bool createConnect();
    bool createTable();

    inline QSqlDatabase database() const { return m_db; }
    Status status() const { return m_status; }
    void setStatus(DatabaseManager::Status status = Read) { m_status = status; }

signals:

public slots:

private:
    QSqlDatabase m_db;
    Status       m_status;
};

#endif // DATABASEMANAGER_H
