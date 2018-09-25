#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>

namespace Ui {
    class logindlg;
}

class logindlg : public QDialog
{
    Q_OBJECT

public:
    explicit logindlg(QWidget *parent = 0);
    ~logindlg();

private:
    Ui::logindlg *ui;

private slots:

    void on_loginBtn_clicked();
};

#endif // LOGINDLG_H
