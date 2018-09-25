#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QTableView>
#include <QSplitter>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QAbstractTableModel>
namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private:
		Ui::MainWindow *ui;

		QLineEdit *find_textLineEdit;
		QDialog *findDlg;
                QPushButton *open_Btn;
                QPushButton *get_Btn;
                QCheckBox *chbox_smbios;
                QCheckBox *chbox_reboot;



        private slots:
                void on_action_N_triggered();
                void on_action_0_triggered();
                void on_action_ID_triggered();
                void on_action_BIOS_triggered();
                void on_action_ID_2_triggered();

		void on_treeWidget_clicked(QModelIndex index);

                bool updata_database(bool);
		void timerEvent(QTimerEvent *);
                void get_bios_data_to_file();
                void choose_rom_file1();
                void choose_rom_file2();
                void choose_rom_file3();
                void choose_rom_file4();
                void choose_rom_file5();
                void choose_rom_file6();
                void choose_rom_file7();
                void choose_rom_file8();
		void update_rate();
                void flash_bios_all();
                void flash_bios_main();
                void flash_bios_boot();
                void flash_bios_nvram();
                void flash_rom_hole();
                void flash_OA2();
                void flash_OA3();

                void get_database_data();
                void set_browser_text();
                void show_dir();
                void show_file();

                void shutdown_system();
                void reboot_system();
                void shutdown_system_now();
                void reboot_system_now();


};

class MyTableModel: public QSqlTableModel
{
    Q_OBJECT
         public:
               explicit MyTableModel(QObject *parent = 0);
               QVariant data(const QModelIndex & index, int role) const;

};



#endif // MAINWINDOW_H
