#include <QtGui/QApplication>
#include <QtGui>
#include <QtSql>
#include <QTextCodec>

#include "mainwindow.h"
#include "connection.h"
#include "logindlg.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
        QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

	if (!createConnection())
		return 1;

	MainWindow w;
	logindlg login;

       if(login.exec()==QDialog::Accepted)
	{
                w.show();
                return app.exec();
	}
        else
            return 0;

}
