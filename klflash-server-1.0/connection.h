#ifndef DATABASE_H
#define DATABASE_H
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>

static bool createConnection()
{
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        QString dbname = "klflashdb";
        db.setHostName("localhost");
        db.setUserName("root");	//  用户名
        db.setPassword("");	// 密码

        bool sr;
        QSqlQuery query(db);
        QSqlDatabase db1 = QSqlDatabase::database();

        sr = query.exec("CREATE DATABASE IF NOT EXISTS " + dbname);
        if (!sr) {
                QMessageBox::warning(0, QObject::tr("Database Error"),
                                     QObject::tr
                                     (" Create Database Error!"));
                return false;
        }

        db.setDatabaseName(dbname);
        if (!db.open()) {
                QMessageBox::critical(0, qApp->tr("Cannot open database"),
                                      qApp->tr
                                      ("Unable to establish a database connection."),
                                      QMessageBox::Cancel);
                return false;
        }

        sr = query.exec("USE " + dbname);
        if (!sr) {
                QMessageBox::warning(0, QObject::tr("Database Error"),
                                     QObject::tr
                                     ("  Database Open Error!"));
                return false;
        }

        sr = query.exec(QObject::tr
                ("CREATE TABLE IF NOT EXISTS flashinfo (successed INT(4) NOT NULL, failed INT(4) NOT NULL,PRIMARY KEY(successed,failed))"));

        if (!sr) {
                QMessageBox::warning(0, QObject::tr("Table Error"),
                                     QObject::tr
                                     ("  Creat Table flashinfo Error!"));
                return false;
        }

              query.exec("select * from flashinfo ");
              query.next();
              if (query.value(0).toString() == "") {
                      sr = query.exec(QObject::trUtf8
                                      (" INSERT INTO flashinfo select 0,0"));
               }
              if (!sr) {
                      QMessageBox::warning(0, QObject::tr("Insert Error"),
                                           QObject::tr
                                           ("  Insert into Table  Error!"));
                      return false;
              }

	return true;
}
#endif				// DATABASE_H
