#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QMessageBox>
#include <QSqlError>
#include <QtDebug>
#include <QSlider>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QCompleter>
#include <QSqlQuery>
#include <fcntl.h>

#include "bios_app.h"


CommandLineOpt opt;
int timerid = 0;
unsigned long count = 0;
QString browsertext;
QString storename = "/klimage.bin";

MyTableModel::MyTableModel(QObject *parent):
    QSqlTableModel(parent)
{


}

QVariant MyTableModel::data(const QModelIndex & index,
        int role = Qt::DisplayRole) const
{
   if (!index.isValid())
        return QVariant();

    QVariant vt = QSqlTableModel::data(index, role);
    if (QVariant::Date == vt.type())
        return vt.toDate().toString("yyyy-MM-dd");
    return vt;

 }



MainWindow::MainWindow(QWidget * parent):
QMainWindow(parent), ui(new Ui::MainWindow)
{
        ui->setupUi(this);

        setWindowTitle("昆仑工具管理端界面");
#if 0
        ui->splitter->addWidget(ui->treeWidget);
        ui->splitter->addWidget(ui->tableView);
        //ui->splitter->addWidget();
      ui->splitter->addWidget(ui->label);
      ui->splitter->addWidget(ui->label_2);
        ui->splitter->addWidget(ui->label_3);
        ui->splitter->addWidget(ui->lineEdit);
        ui->splitter->addWidget(ui->lineEdit_2);
        ui->splitter->addWidget(ui->lineEdit_3);
#endif
         ui->splitter->setStretchFactor(0, 1);	//主窗口分成两半
         ui->splitter_2->setStretchFactor(1,1);
         ui->splitter->show();
         ui->splitter_2->show();

         get_database_data();
         //ui->textBrowser
}

MainWindow::~MainWindow()
{
	delete ui;
}


int static flags = 1;		//用于每个对话框间的互斥

void MainWindow::on_treeWidget_clicked(QModelIndex index)
{

        if (index.data().toString() == "获取BIOS数据") {
            get_bios_data_to_file();
        } else if (index.data().toString() == "刷新全部BIOS") {
            flash_bios_all();
        } else if (index.data().toString() == "刷新BIOS主块") {
            flash_bios_main();
        } else if (index.data().toString() == "刷新BIOS启动块") {
            flash_bios_boot();
        } else if (index.data().toString() == "刷新NVRAM") {
            flash_bios_nvram();
        } else if (index.data().toString() == "刷新ROM Holes") {
            flash_rom_hole();
        } else if (index.data().toString() == "刷新OA2") {
            flash_OA2();
        } else if (index.data().toString() == "刷新OA3") {
            flash_OA3();
        } else if (index.data().toString() == "重启系统") {
            reboot_system();
        } else if (index.data().toString() == "关闭系统") {
            shutdown_system();
        }

}





void MainWindow::on_action_BIOS_triggered()
{
    QMessageBox::about(this, tr("About KunLun BIOS Tools"),
                       tr("<h2>KunLun Firmware Update Utility V1.0 </h2>"
                          "<p>Copyright &copy; 2010-2014 ZD Tech (Beijing) Ltd. All Rights Reserved"
                          "<p>KunLun BIOS Tools is a small application that "
                          "gather the FLASH tool and SMI tool."
                          "and some operations about program the flash,"
                          "and show the progress to users."));
}

/*
 *  更新数据库
*/
bool MainWindow::updata_database(bool ok)
{
    bool sr;
    QSqlQuery query;
    if (ok == true){
        sr = query.exec("update flashinfo set successed = successed+1");
        get_database_data();
    }else{
        sr = query.exec("update flashinfo set successed = failed+1");
        get_database_data();
    }
    if (!sr) {
            QMessageBox::warning(0, QObject::tr("Update database"),
                                 QObject::tr
                                 (" Upadate Database Error!"));
            return false;
    }

            return true;
}

void MainWindow::update_rate()
{
        bool ok = false;
       // qDebug()<<"----------------"<< opt.rate;
	ui->progressBar->setValue(opt.rate);
        set_browser_text();

        if(opt.rate == 100){
            killTimer(timerid);
            ui->progressBar->setValue(opt.rate);
            count = 0;
            flags = 1;
            ok = true;
            updata_database(ok);
        }



}

void MainWindow::timerEvent(QTimerEvent *event)                     //定时器事件
{
  	int id = event->timerId();
        //qDebug() << id;
	if (id == timerid)
             update_rate();
}
  
void MainWindow::get_bios_data_to_file()
{
    if (flags == 1) {
#if 1
            findDlg = new QDialog(this);                                        //新建一个对话框,用于查找操作,this 表明它的父窗口是 MainWindow。
            findDlg->setWindowTitle(tr("选择存取文件位置"));                     //设置对话框的标题
            find_textLineEdit = new QLineEdit(findDlg);                         //将行编辑器加入到新建的查找对话框中


            QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);	//加入一个“获取”的按钮
            get_Btn = new QPushButton(tr("获取数据"), findDlg);

            QLabel *where = new QLabel(findDlg);
            where->setText("位置:");

            QHBoxLayout *layout = new QHBoxLayout;
            layout->addWidget(where);
            layout->addWidget(find_textLineEdit);

            QFormLayout *formLayout = new QFormLayout(findDlg);

            // formLayout->addRow(tr("&位置:"), find_textLineEdit);
   \
             formLayout->addRow(layout);
             formLayout->addRow(find_Btn, get_Btn);
             formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
             formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
             //formLayout->setLabelAlignment(Qt::AlignLeft);
             //formLayout->setHorizontalSpacing(50);

             findDlg->setLayout(formLayout);

            //find_textLineEdit->setDisabled(true);
            //find_textLineEdit->setMaxLength(100);
            find_textLineEdit->setReadOnly(true);
            find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
            find_textLineEdit->adjustSize();
            find_textLineEdit->autoFillBackground();
            findDlg->show();

            connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file1()));	//设置“查找”按钮的单击事件和其槽函数的关联
            connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_dir()));

            flags++;
            if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
                  //  qDebug()<<"关闭窗口";
                    flags = 1;
            }
#endif

    }
}

void MainWindow::flash_bios_all()
{
    if (flags == 1) {
        findDlg = new QDialog(this);                                        //新建一个对话框,用于查找操作,this 表明它的父窗口是 MainWindow。
        findDlg->setWindowTitle(tr("选择烧写文件位置"));                     //设置对话框的标题
        find_textLineEdit = new QLineEdit(findDlg);                         //将行编辑器加入到新建的查找对话框中


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);	//加入一个“获取”的按钮
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        chbox_smbios = new QCheckBox();
        chbox_smbios->setText("保留SMBIOS");
        chbox_smbios->setCheckState(Qt::Unchecked);

        chbox_reboot = new QCheckBox();
        chbox_reboot->setText("刷完后重启");
        chbox_reboot->setCheckState(Qt::Unchecked);

        QFormLayout *formLayout = new QFormLayout(findDlg);
        formLayout->addRow(layout);
        formLayout->addRow(chbox_smbios, chbox_reboot);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
         //formLayout->setLabelAlignment(Qt::AlignLeft);
         //formLayout->setHorizontalSpacing(50);

        findDlg->setLayout(formLayout);

        //find_textLineEdit->setDisabled(true);
        //find_textLineEdit->setMaxLength(100);
        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file2()));	//设置“查找”按钮的单击事件和其槽函数的关联
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::flash_bios_main()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file3()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::flash_bios_boot()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        chbox_smbios = new QCheckBox();
        chbox_smbios->setText("保留SMBIOS");
        chbox_smbios->setCheckState(Qt::Unchecked);

        chbox_reboot = new QCheckBox();
        chbox_reboot->setText("刷完后重启");
        chbox_reboot->setCheckState(Qt::Unchecked);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(chbox_smbios, chbox_reboot);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file4()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::flash_bios_nvram()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file5()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::flash_rom_hole()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        chbox_smbios = new QCheckBox();
        chbox_smbios->setText("保留SMBIOS");
        chbox_smbios->setCheckState(Qt::Unchecked);

        chbox_reboot = new QCheckBox();
        chbox_reboot->setText("刷完后重启");
        chbox_reboot->setCheckState(Qt::Unchecked);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(chbox_smbios, chbox_reboot);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file6()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
               // qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::flash_OA2()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file7()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}


void MainWindow::flash_OA3()
{
    if (flags == 1) {
        findDlg = new QDialog(this);
        findDlg->setWindowTitle(tr("选择烧写文件位置"));
        find_textLineEdit = new QLineEdit(findDlg);


        QPushButton *find_Btn = new QPushButton(tr("打开..."), findDlg);
        get_Btn = new QPushButton(tr("开始烧写"), findDlg);

        QLabel *where = new QLabel(findDlg);
        where->setText("位置:");

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(where);
        layout->addWidget(find_textLineEdit);

        QFormLayout *formLayout = new QFormLayout(findDlg);

        formLayout->addRow(layout);
        formLayout->addRow(find_Btn, get_Btn);
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);


        findDlg->setLayout(formLayout);

        find_textLineEdit->setReadOnly(true);
        find_textLineEdit->setToolTip(QObject::trUtf8("例如:/opt/aa.bin"));
        find_textLineEdit->adjustSize();
        find_textLineEdit->autoFillBackground();
        findDlg->show();

        connect(get_Btn, SIGNAL(clicked()), this, SLOT(choose_rom_file8()));
        connect(find_Btn, SIGNAL(clicked()), this, SLOT(show_file()));



        flags++;
        if (findDlg->exec() == QDialog::Rejected || opt.rate ==100) {
              //  qDebug()<<"关闭窗口";
                flags = 1;
        }
    }
}

void MainWindow::reboot_system()
{
    if (flags == 1)
    {

        int ret = QMessageBox::warning(this,tr("Warning"),tr("确定要重启计算机？？"),
                                       QMessageBox::No|QMessageBox::Yes,QMessageBox::Yes);
        flags++;
        if(QMessageBox::No == ret)
        {
            flags = 1;
            return;
        }
        else if (QMessageBox::Yes == ret)
        {
           // qDebug()<<"将要重启系统";
            reboot_system_now();
        }

        return;

    }
}

void MainWindow::shutdown_system()
{
    if (flags == 1)
    {

        int ret = QMessageBox::warning(this,tr("Warning"),tr("确定要关闭计算机？？"),
                                       QMessageBox::No|QMessageBox::Yes,QMessageBox::Yes);
        flags++;
        if(QMessageBox::No == ret)
        {
            flags = 1;
            return;
        }
        else if (QMessageBox::Yes == ret)
        {
           // qDebug()<<"将要关闭系统";
            shutdown_system_now();
        }

        return;

    }
}

void MainWindow::reboot_system_now()
{
    memset(&opt, 0, sizeof(opt));

    strcpy(opt.command, "/reboot");
    opt.hasCommand = 1;

    if (smi_interface(&opt))
        return;

    return;
}

void MainWindow::shutdown_system_now()
{
    memset(&opt, 0, sizeof(opt));

    strcpy(opt.command, "/shutdown");
    opt.hasCommand = 1;

    if (smi_interface(&opt))
        return;

    return;

}

void MainWindow::choose_rom_file1()
{

    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    strcpy(opt.command, "/o");
    opt.fileNameLength = strlen(opt.fileName);
    opt.hasCommand = 1;
    opt.rate = 0;

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }

    return;
}

void MainWindow::choose_rom_file2()
{

    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/all");
    opt.rate = 0;

    int ischeckable_smbios = 0;
    int ischeckable_reboot = 0;

    if (chbox_smbios->isChecked())
    {
        ischeckable_smbios = 1;
    }

    if (chbox_reboot->isChecked())
    {
       ischeckable_reboot = 1;
    }

    if (ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 3;
        strcpy(opt.options[1],"/R");
        strcpy(opt.options[2],"/reboot");
    }
    else if (ischeckable_smbios && !ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/R");
    }
    else if (!ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/reboot");
    }

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }

    return;

}

void MainWindow::choose_rom_file3()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/p");
    opt.rate = 0;

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}

void MainWindow::choose_rom_file4()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/b");
    opt.rate = 0;

    int ischeckable_smbios = 0;
    int ischeckable_reboot = 0;

    if (chbox_smbios->isChecked())
    {
        ischeckable_smbios = 1;
    }

    if (chbox_reboot->isChecked())
    {
       ischeckable_reboot = 1;
    }

    if (ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 3;
        strcpy(opt.options[1],"/R");
        strcpy(opt.options[2],"/reboot");
    }
    else if (ischeckable_smbios && !ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/R");
    }
    else if (!ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/reboot");
    }


    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}

void MainWindow::choose_rom_file5()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/N");
    opt.rate = 0;

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}

void MainWindow::choose_rom_file6()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/L");
    opt.rate = 0;


    int ischeckable_smbios = 0;
    int ischeckable_reboot = 0;

    if (chbox_smbios->isChecked())
    {
        ischeckable_smbios = 1;
    }

    if (chbox_reboot->isChecked())
    {
       ischeckable_reboot = 1;
    }

    if (ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 3;
        strcpy(opt.options[1],"/R");
        strcpy(opt.options[2],"/reboot");
    }
    else if (ischeckable_smbios && !ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/R");
    }
    else if (!ischeckable_smbios && ischeckable_reboot)
    {
        opt.optionNumber = 2;
        strcpy(opt.options[1],"/reboot");
    }


    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}

void MainWindow::choose_rom_file7()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/OA2");
    opt.rate = 0;

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}

void MainWindow::choose_rom_file8()
{
    memset(&opt, 0, sizeof(opt));
    QString findText = find_textLineEdit->text();

    if (findText == "") {
           QMessageBox::warning(0, QObject::tr("Error"),
                             QObject::tr
                             (" 请选择存储文件路径!"));

        return;
    }

    timerid = startTimer(500);

    QByteArray ba = findText.toAscii();
    strcpy(opt.fileName, ba.data());
    opt.fileNameLength = strlen(opt.fileName);
    opt.optionNumber = 1;
    strcpy(opt.options[0],"/OA3");
    opt.rate = 0;

    findDlg->close();

    if (smi_interface(&opt))
        return;

/*子线程在线程之前结束的操作*/
    if(opt.rate == 100){
        flags = 1;
        killTimer(timerid);
    }


    return;

}


void MainWindow::on_action_ID_2_triggered()
{
     get_bios_data_to_file();
}


void MainWindow::set_browser_text()
{
    unsigned int i = 0;
    QString addtext;

    if (count == 0){
        browsertext = "开始......\n";
        browsertext.append("文件名为:");
        browsertext.append(opt.fileName);
        browsertext.append("\n=0\n");
        ui->textBrowser->insertPlainText(browsertext);

    }
    count++;
    for(i = 0; i < count+1; i++)
    {
        addtext.append("=");
    }
    addtext.append(QString::number(opt.rate));
    addtext.append("\n");


    if(opt.rate == 100){
        addtext.append("完成...................................................................\n\n");
    }


    ui->textBrowser->insertPlainText(addtext);
}
#if 0
void MainWindow::set_browser_text()
{
    unsigned int i = 0;
    QString addtext;
    if (count == 0){
        browsertext = "开始......\n";
        browsertext.append("文件名为:");
        browsertext.append(opt.fileName);
        browsertext.append("\n");

    }
    count++;
    for(i = 0; i < count; i++)
    {
        browsertext.append("=");
    }
    browsertext.append(QString::number(opt.rate));
    browsertext.append("\n");


    if(opt.rate == 100){
        browsertext.append("完成......\n");
    }

    ui->textBrowser->setText(browsertext);
   // ui->textBrowser->insertPlainText(browsertext);
}
#endif
void MainWindow::get_database_data()
{

    QSqlQuery query;
    query.exec("select * from flashinfo ");
    query.next();
    ui->lineEdit->setText(query.value(0).toString());
    ui->lineEdit_2->setText(query.value(1).toString());
    int sum = query.value(0).toInt()+query.value(1).toInt();
    ui->lineEdit_3->setText(QString::number(sum));
}

void MainWindow::show_dir()
{
#if 0
    QString fileFull = "/";
    QFileInfo fi=QFileInfo(fileFull);
    QString filePath;
    filePath=fi.absolutePath();

    QDesktopServices::openUrl(QUrl(filePath, QUrl::TolerantMode));

#endif

     QList<QUrl> urls;
    //  urls << QUrl::fromLocalFile("./")
     urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation))
          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

      QFileDialog dialog;
      dialog.setSidebarUrls(urls);
      dialog.setFileMode(QFileDialog::DirectoryOnly);

      if(dialog.exec()) {
          QString file = dialog.selectedFiles().at(0);

          file = file+storename;
          find_textLineEdit->setText(file);

          QFileInfo info(file);

        //  ()<<"path is "<<info.path();
        //  qDebug()<<"file is "<<file;


      }

}

void MainWindow::show_file()
{
     QList<QUrl> urls;
     urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation))
          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

      QFileDialog dialog;
      dialog.setSidebarUrls(urls);
      dialog.setFileMode(QFileDialog::AnyFile);
      if(dialog.exec()) {
          QString file = dialog.selectedFiles().at(0);
          QFileInfo info(file);

          find_textLineEdit->setText(file);
         // qDebug()<<"path is "<<info.path();
         // qDebug()<<"file is "<<file;


      }
}


void MainWindow::on_action_ID_triggered()
{
    flash_bios_all();
}

void MainWindow::on_action_0_triggered()
{
    flash_bios_boot();
}

void MainWindow::on_action_N_triggered()
{
     flash_bios_main();
}
