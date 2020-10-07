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
    if (useJSONasInput)
    {
        QFile file (dbFilename);
        if (file.exists())
            file.remove();
    }
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
                m_db.setDatabaseName(dbFilename);
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
                m_db.setDatabaseName(dbFilename);
            }
        };
        break;
    }

    if( !m_db.isOpen() )
    {
        if ( !m_db.open() )
        {
            qDebug()<< QString(" can't open database >>>>>> database.dat");
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
        //QString createTable = QString("CREATE TABLE %1 (ID integer PRIMARY KEY AUTOINCREMENT,"
        //                              "Checked integer default 0, LayerName text, Thickness text, Density text, Vs text, Material text,ElementSize text, Color text, FEM text)"
        //                              ).arg(g_dbTableName);
        QString createTable = QString("CREATE TABLE %1 (ID integer PRIMARY KEY AUTOINCREMENT,"
                                      "Checked integer default 0, LayerName text default Soil, Thickness text default 3.0, Density text default 2.0, Vs text default 180, Material text default Elastic, ElementSize text default 0.25, Color text, FEM text)"
                                      ).arg(g_dbTableName);
        //QString createTable = QString("CREATE TABLE %1 (LayerID text,"
        //                              "Checked integer default 0, LayerName text default DefaultSand, Thickness text default 3.0, Density text default 2.0, Vs text default 180, Material text default Elastic, ElementSize text default 0.25, Color text, FEM text)"
        //                              ).arg(g_dbTableName);
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
