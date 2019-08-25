#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void show_system_info();
    void show_process_info();
    void show_capability();
    void paintEvent(QPaintEvent *e);

private slots:
    void on_action_2_triggered();
    void on_action_3_triggered();
    void on_action_4_triggered();
    void on_action_5_triggered();
    void time_out_update_current_tab_info();
    void time_out_update_status_bar();
    void show_tab_info(int index);
    void on_tabWidget_currentChanged(int index);
    void on_pushButton_Refresh_clicked();
    void on_pushButton_KillPro_clicked();
    void on_pushButton_Search_clicked();
    void on_action_triggered();

private:
    Ui::MainWindow *ui;
    QTimer *timer;//定时器

    int total0,idle0,total1,idle1;
    float cpuUsed;
    float memUsed;

    QPixmap pixmap[3];
    int columnInc[3]; // 动态列偏移值
    float *pts[3]; // 数据数组
    int WLenBefore[3]; // 记录前一时刻pixmap的宽度
    int ptNum[3]; // 点数目
    float value[3];//当前数据数组
};

#endif // MAINWINDOW_H
