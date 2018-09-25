#include "logindlg.h"
#include "ui_logindlg.h"
#include <QtGui>

logindlg::logindlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::logindlg)
{
	ui->setupUi(this);
//	ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
}

logindlg::~logindlg()
{
	delete ui;
}

void logindlg::on_loginBtn_clicked()
{
    int ret = QMessageBox::warning(this,tr("Warning"),tr("您正在试图操作BIOS,请慎重操作！禁止断电！\n                 是否要继续?"),
                                   QMessageBox::No|QMessageBox::Yes,QMessageBox::Yes);
    if(QMessageBox::No == ret)
        return;
    else
        accept();
}
