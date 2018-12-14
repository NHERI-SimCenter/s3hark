#include "DatabaseManager.h"
#include "GlobalConstances.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QMessageBox>

#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent),
    m_status(Read)
{
}


bool DatabaseManager::createConnect()
{
    switch( m_status )
    {
    case Read:
        {
            if(QSqlDatabase::contains("READ"))
                m_db = QSqlDatabase::database("READ");
            else
            {
                m_db = QSqlDatabase::addDatabase("QSQLITE", "READ");
                m_db.setDatabaseName(g_dbFileName);
            }
        };
        break;
    case Write:
        {
            if(QSqlDatabase::contains("WRITE"))
                m_db = QSqlDatabase::database("WRITE");
            else
            {
                m_db = QSqlDatabase::addDatabase("QSQLITE", "WRITE");
                m_db.setDatabaseName(g_dbFileName);
            }
        };
        break;
    }

    if( !m_db.isOpen() )
    {
        if ( !m_db.open() )
        {
            qDebug()<< QString(" can't open database >>>>>> data.sqlite");
            qDebug() << "error code: " << m_db.lastError();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::createTable()
{
    QStringList tableList = m_db.tables();
    QSqlQuery query(m_db);
    if( !tableList.contains(g_dbTableName) )
    {
        QString createTable = QString("CREATE TABLE %1 (ID integer PRIMARY KEY AUTOINCREMENT,"
                                      "Checked integer default 0, LayerName text, Thickness text, Density text, Vs text, Material text, Color text, FEM text)"
                                      ).arg(g_dbTableName);
        if(!query.exec(createTable))
        {
            QMessageBox::warning(0, QObject::tr("Create table error"), QObject::tr("Error: ")+ query.lastError().text(), QMessageBox::Ok);
            qDebug()<< "Create table error: " << query.lastError();
            return false;
        }
    }

    /*
    if( !tableList.contains(g_dbFEMTableName) )
    {
        QString createTable = QString("CREATE TABLE %1 (ID integer PRIMARY KEY AUTOINCREMENT,"
                                      "Checked integer default 0, eSizeH text, eSizeV text)"
                                      ).arg(g_dbFEMTableName);
        if(!query.exec(createTable))
        {
            QMessageBox::warning(0, QObject::tr("Create table error"), QObject::tr("Error: ")+ query.lastError().text(), QMessageBox::Ok);
            qDebug()<< "Create table error: " << query.lastError();
            return false;
        }
    }
    */

    return true;
}
