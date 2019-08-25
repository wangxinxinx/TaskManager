#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QInputDialog>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(time_out_update_current_tab_info()));
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(time_out_update_status_bar()));
    timer->start(1000);//每1s触发一次定时器

    show_tab_info(0);

    ui->tableWidget_Process->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑表格
    ui->tableWidget_Process->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    ui->tableWidget_Process->setSelectionMode(QAbstractItemView::SingleSelection); //只能选中单个目标
    ui->tableWidget_Process->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置表格为自动扩展
    ui->tableWidget_Process->setRowCount(500);
    ui->tableWidget_Process->setColumnCount(6);

    total0=0;
    idle0=0;
    total1=0;
    idle1=0;

    for(int i=0; i<3;i++)
    {
        pts[i]=NULL;
        columnInc[i]=10;
        value[i] = 0;
        WLenBefore[i] = 504;
        pixmap[i] = QPixmap(504,40);
        pixmap[i].fill(Qt::black); // 黑色背景图
    }

    ui->label_cpuUsage->setScaledContents(true);
    ui->label_MemUsage->setScaledContents(true);
    ui->label_SwapUsage->setScaledContents(true);

    ui->label_cpuUsage->setPixmap(pixmap[0]);
    ui->label_MemUsage->setPixmap(pixmap[1]);
    ui->label_SwapUsage->setPixmap(pixmap[2]);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
    for(int i=0;i<3;i++)
    {
        if(pts[i]) free(pts[i]);
    }
}

//退出程序
void MainWindow::on_action_2_triggered()
{    
    qApp->quit();
}

//关机
void MainWindow::on_action_3_triggered()
{   
    //system("halt");
    system("shutdown -h now");
}

//重启
void MainWindow::on_action_4_triggered()
{   
   // system("reboot");
    system("shutdown -r now");
}

//挂起
void MainWindow::on_action_5_triggered()
{
    int value = QInputDialog::getInt(this,"挂起",tr("挂起时间(s)"),1,1);
    QString susp = tr("rtcwake -m mem -s %1").arg(value);
    system(susp.toLatin1());
}

//显示系统信息
void MainWindow::show_system_info()
{
    QString Str; //读取文件信息字符串
    QFile File; //用于打开系统文件
    int pos; //读取文件的位置

    File.setFileName("/proc/sys/kernel/hostname");
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\" /proc/sys/kernel/hostname\"打开失败!"), QMessageBox::Yes);
        return ;
    }
    Str = File.readLine();
    QString host_name(Str.mid(0, Str.length()-1));
    ui->label_HostName->setText(host_name);
    File.close();

    File.setFileName("/proc/uptime");
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\" /proc/uptime\"打开失败!"), QMessageBox::Yes);
        return ;
    }
    Str = File.readLine();
    pos = Str.indexOf(" ");
    QString runned_time( Str.mid(0,pos-3) );//只取整数部分

    time_t t_runned_time = runned_time.toLong();
    int runned_second = t_runned_time%60;
    int runned_minute = (t_runned_time/60)%60;
    int runned_hour = (t_runned_time/3600)%60;
    runned_time = tr("%1时%2分%3秒").arg(runned_hour).arg(runned_minute).arg(runned_second);
    ui->label_RunnedTime->setText(runned_time );

    time_t cur_time = time(NULL);
    time_t t_boot_time = cur_time - t_runned_time;
    struct tm *ptm = localtime(&t_boot_time);
    QString boot_time = tr("%1-%2-%3 %4:%5:%6").arg(ptm->tm_year + 1900).arg(ptm->tm_mon + 1).arg( ptm->tm_mday).arg(ptm->tm_hour).arg(ptm->tm_min).arg(ptm->tm_sec);
    ui->label_BootTime->setText(boot_time );

    File.close();

    File.setFileName("/proc/version");
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\"/proc/version\"打开失败!"), QMessageBox::Yes);
        return ;
    }

    Str = File.readLine();
    pos = Str.indexOf("version");
    QString os_type( Str.mid(0, pos-1) );
    ui->label_osType->setText(os_type);

    int pos1 = Str.indexOf("(");
    QString os_version( Str.mid(pos, pos1-pos-1) );
    ui->label_osVersion->setText(os_version);

    pos = Str.indexOf("gcc version");
    pos1 = Str.indexOf("#");
    QString gcc_version( Str.mid(pos+12, pos1-pos-14) );
    ui->label_gccVersion->setText(gcc_version);

    File.close();

    File.setFileName("/proc/cpuinfo");
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\"/proc/cpuinfo\"打开失败!"), QMessageBox::Yes);
        return;
    }

    //循环读取文件内容，查找需要的信息
    while(1)
    {
        Str = File.readLine();

        if (pos = Str.indexOf("model name"),pos != -1)
        {
            pos += 13; //跳过前面的"model name："所占用的字符
            QString cpu_model_name ( Str.mid(pos, Str.length()-14) );//还要去掉行尾的换行符
            ui->label_cpuModelName->setText(cpu_model_name);
        }
        else if (pos = Str.indexOf("vendor_id"), pos != -1)
        {
            pos += 12; //跳过前面的"vendor_id："所占用的字符
            QString cpu_type( Str.mid(pos, Str.length()-13) );
            ui->label_cpuType->setText(cpu_type);
        }
        else if (pos = Str.indexOf("cpu MHz"), pos != -1)
        {
            pos += 11; //跳过前面的"cpu MHz："所占用的字符
            QString cpu_frequency( Str.mid(pos, Str.length()-12) );
            ui->label_cpuFrequency->setText(cpu_frequency + " MHz");
        }
        else if (pos = Str.indexOf("cache size"), pos!=-1)
        {
            pos += 13; //跳过前面的"cache size："所占用的字符
            QString cache_size( Str.mid(pos, Str.length()-14) );
            ui->label_CacheSize->setText(cache_size);
            break;
        }
    }
    File.close();
}

//显示进程信息
void MainWindow::show_process_info()
{
    bool ok;
    int find_start = 3;
    int a, b;
    int n_pro_pid; //进程pid
    int row_count = 0;
    int total_pro_num = 0,number_of_sleep = 0, number_of_run = 0, number_of_zombie = 0;

    QString Str; //读取文件信息字符串
    QFile File; //用于打开系统文件

    QString pro_pid;//进程id
    QString pro_name; //进程名
    QString pro_ppid;//父进程id
    QString pro_state; //进程状态
    QString pro_pri; //进程优先级
    QString pro_mem; //进程占用内存

    QDir qd("/proc");
    QStringList qsList = qd.entryList();//获取/proc目录下的所有文件和目录
    QString qs = qsList.join("\n");//将获取的列表转换成QString类型，项之间用'\n'分隔

    ui->tableWidget_Process->clear();

    //循环读取进程
    while (1)
    {
        //获取pid
        a = qs.indexOf("\n", find_start);
        b = qs.indexOf("\n", a+1);
        find_start = b;
        pro_pid = qs.mid(a+1, b-a-1);
        n_pro_pid = pro_pid.toInt(&ok, 10);
        if(!ok)
        {//如果转换失败，说明不是进程文件夹
            break;
        }
        total_pro_num++;

        //打开pid所对应的进程状态文件
        File.setFileName("/proc/" + pro_pid + "/stat");
        if ( !File.open(QIODevice::ReadOnly) )
        {
            QMessageBox::warning(this, tr("警告"), tr("文件\"/proc/%1/stat\"打开失败!").arg(pro_pid), QMessageBox::Yes);
            return;
        }
        Str = File.readLine();
        if (Str.length() == 0)
        {
            break;
        }
        a = Str.indexOf("(");
        b = Str.indexOf(")");
        pro_name = Str.mid(a+1, b-a-1);
        pro_name.trimmed(); //删除两端的空格
        pro_state = Str.section(" ", 2, 2);//以空格为分隔的第2到第2部分的字符串
        pro_ppid = Str.section(" ", 3, 3);
        pro_pri = Str.section(" ", 17, 17);
        pro_mem = Str.section(" ", 23, 23);//以页为单位，每页4KB
        int i_pro_mem = pro_mem.toInt();
        pro_mem.setNum(i_pro_mem*4);

        switch ( pro_state.at(0).toLatin1() )
        {
            case 'S':   number_of_sleep++; break; //Sleep
            case 'R':   number_of_run++; break; //Running
            case 'Z':   number_of_zombie++; break; //Zombie
            default :   break;
        }

      QStringList title_list;
      title_list<<"pid"<<"名称"<<"状态"<<"ppid"<<"优先级"<<"占用内存(KB)";
      ui->tableWidget_Process->setHorizontalHeaderLabels(title_list);

      ui->tableWidget_Process->setItem(row_count, 0, new QTableWidgetItem(pro_pid));
      ui->tableWidget_Process->setItem(row_count, 1, new QTableWidgetItem(pro_name));
      ui->tableWidget_Process->setItem(row_count, 2, new QTableWidgetItem(pro_state));
      ui->tableWidget_Process->setItem(row_count, 3, new QTableWidgetItem(pro_ppid));
      ui->tableWidget_Process->setItem(row_count, 4, new QTableWidgetItem(pro_pri));
      ui->tableWidget_Process->setItem(row_count, 5, new QTableWidgetItem(pro_mem));
      row_count++;

      File.close();
    }
    ui->tableWidget_Process->setRowCount(row_count);

    QString temp;
    temp = QString::number(total_pro_num, 10);
    ui->label_pNum->setText(temp);
    temp = QString::number(number_of_run, 10);
    ui->label_pRun->setText(temp);
    temp = QString::number(number_of_sleep, 10);
    ui->label_pSleep->setText(temp);
    temp = QString::number(number_of_zombie, 10);
    ui->label_pZombie->setText(temp);

    File.close(); //关闭该pid进程的状态文件

}

//显示性能信息
void MainWindow::show_capability()
{
    QString Str; //读取文件信息字符串
    QFile File; //用于打开系统文件
    int pos; //读取文件的位置

    File.setFileName("/proc/stat");
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\"/proc/stat\"打开失败!"), QMessageBox::Yes);
        return;
    }
    Str = File.readLine();

    total0 = total1;
    idle0 = idle1;
    total1=0,idle1=0;

    for (int i = 0; i < 9; i++)
    {
        total1 += Str.section(" ", i+2, i+2).toInt();

        if (i == 3)
        {
            idle1 = Str.section(" ", i+2, i+2).toInt();
        }
    }

    int idle, total;
    idle = idle1 - idle0;
    total = total1 - total0;
    if (idle < 0)
    {
        idle = -idle;
    }
    if (total < 0)
    {
        total = -total;
    }

    File.close(); //关闭stat文件

    cpuUsed =(float) (total-idle)/(float)total;
    value[0] = cpuUsed;


    File.setFileName("/proc/meminfo"); //打开内存信息文件
    if ( !File.open(QIODevice::ReadOnly) )
    {
        QMessageBox::warning(this, tr("警告"), tr("文件\"/proc/meminfo\"打开失败!"), QMessageBox::Yes);
        return ;
    }

    QString mem_total;
    QString mem_free;
    QString mem_used;
    QString swap_total;
    QString swap_free;
    QString swap_used;
    int n_mem_total, n_mem_free, n_mem_used, n_swap_total, n_swap_free, n_swap_used;

    while (1)
    {
        Str = File.readLine();
        pos = Str.indexOf("MemTotal");
        if (pos != -1)
        {
            mem_total = Str.mid(pos+10, Str.length()-13);
            mem_total = mem_total.trimmed();
            n_mem_total = mem_total.toInt()/1024;
        }
        else if (pos = Str.indexOf("MemFree"), pos != -1)
        {
            mem_free = Str.mid(pos+9, Str.length()-12);
            mem_free = mem_free.trimmed();
            n_mem_free = mem_free.toInt()/1024;
        }
        else if (pos = Str.indexOf("SwapTotal"), pos != -1)
        {
            swap_total = Str.mid(pos+11, Str.length()-14);
            swap_total = swap_total.trimmed();
            n_swap_total = swap_total.toInt()/1024;
        }
        else if (pos = Str.indexOf("SwapFree"), pos != -1)
        {
            swap_free = Str.mid(pos+10,Str.length()-13);
            swap_free = swap_free.trimmed();
            n_swap_free = swap_free.toInt()/1024;
            break;
        }
    }

    File.close();

    n_mem_used = n_mem_total - n_mem_free;
    n_swap_used = n_swap_total - n_swap_free;

    mem_used = QString::number(n_mem_used, 10);
    mem_free = QString::number(n_mem_free, 10);
    mem_total = QString::number(n_mem_total, 10);
    swap_used = QString::number(n_swap_used, 10);
    swap_free = QString::number(n_swap_free, 10);
    swap_total = QString::number(n_swap_total, 10);

    ui->label_RAMUsed->setText(mem_used+" MB");
    ui->label_RAMFree->setText(mem_free+" MB");
    ui->label_RAMTotal->setText(mem_total+" MB");
    ui->label_SWAPUsed->setText(swap_used+" MB");
    ui->label_SWAPFree->setText(swap_free+" MB");
    ui->label_SWAPTotal->setText(swap_total+" MB");

    memUsed =(float)n_mem_used/(float)n_mem_total;
    value[1] = memUsed;
    value[2] =(float)n_swap_used/(float)n_swap_total;

    update();
    ui->label_cpuUsage->setPixmap(pixmap[0]);
    ui->label_MemUsage->setPixmap(pixmap[1]);
    ui->label_SwapUsage->setPixmap(pixmap[2]);
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    for(int index =0;index<3;index++)
    {
        switch(index)
        {//调整Pixmap的大小
        case 0:
            pixmap[index] = pixmap[index].scaled(ui->label_cpuUsage->width(),ui->label_cpuUsage->height(),Qt::IgnoreAspectRatio);//不保持图片的长宽比
            break;
        case 1:
            pixmap[index] = pixmap[index].scaled(ui->label_MemUsage->width(),ui->label_MemUsage->height(),Qt::IgnoreAspectRatio);
            break;
        case 2:
            pixmap[index] = pixmap[index].scaled(ui->label_SwapUsage->width(),ui->label_SwapUsage->height(),Qt::IgnoreAspectRatio);
            break;
        }

        QPainter painter(&pixmap[index]);//在pixmap上画图
        pixmap[index].fill(Qt::black); // 黑色背景图
        QPen pen(QColor(46, 139, 87),1.0);// 线条色
        painter.setPen(pen);

        int graphW=pixmap[index].width();
        int graphH=pixmap[index].height();
        int graphRow=graphH/5;
        int graphColumn=graphW/15;

        // 画5行横线
        for(int i=0;i<=graphH;i+=graphRow)
        {
            painter.drawLine(0,i,graphW,i);
        }

         // 改变竖线开始的地方,使曲线产生运动效果
        if(columnInc[index] == -1)
        {
            columnInc[index]=graphColumn;
        }        
        columnInc[index]--;

        // 画15列竖线
        for(int j= columnInc[index];j<graphW;j+=graphColumn)
        {
            painter.drawLine(j,0,j,graphH);
        }

        if(pts[index]==NULL)
        {
            pts[index]=(float *)malloc(graphW*sizeof(float));
            ptNum[index]=0;
        }

        if(WLenBefore[index]<graphW)
        {//如果宽度变长
            pts[index] = (float *)realloc((float *)pts[index],graphW*sizeof(float));
        }
        else if(WLenBefore[index]>graphW)
        {//如果宽度变短
            if(ptNum[index]>graphW) // 当前点数目大于当前宽度
            {
                int sub = ptNum[index] - graphW;
                for(int i=sub;i<ptNum[index];i++)
                {
                    pts[index][i-sub]=pts[index][i];  // 舍弃最前面sub点
                }
                ptNum[index] -= sub;
            }
            else
            {
                pts[index] = (float *)realloc((float *)pts[index],graphW*sizeof(float));
            }
        }

        //记录使用情况
        pts[index][ptNum[index]]=1-value[index];

        // 遍历数组,根据数组中的值画曲线
        painter.setPen(QPen(Qt::yellow,1.0));
        for(int j=graphW-ptNum[index],k=4;k<ptNum[index];k++,j++)
        {
                painter.drawLine(j-1,pts[index][k-1]*graphH,j,pts[index][k]*graphH); // 画直线
        }

        if(ptNum[index]+1>=graphW) // 曲线到达最左端
        {
            int sub = ptNum[index] +1 - graphW;
            for(int i=sub+1;i<ptNum[index];i++)
            {
                pts[index][i-sub-1]=pts[index][i];  // 舍弃最前面的点,从后向前赋值
            }
            ptNum[index] -= sub+1;
        }
        else // 曲线在中间
        {
            ptNum[index]++;
        }

        WLenBefore[index]=graphW;
    }
}

void MainWindow::show_tab_info(int index)
{
    switch(index)
    {
        case 0://系统信息
        show_system_info();
        break;
        case 1://进程信息
        show_process_info();
        break;
        case 2://性能
        show_capability();
        break;
    }
}


void MainWindow::time_out_update_current_tab_info()
{
    show_capability();
}

void MainWindow::time_out_update_status_bar()
{
    time_t t_cur_time = time(NULL);
    struct tm *ptm = localtime(&t_cur_time);
    QString cur_time = tr("%1-%2-%3 %4:%5:%6").arg(ptm->tm_year + 1900).arg(ptm->tm_mon + 1).arg( ptm->tm_mday).arg(ptm->tm_hour).arg(ptm->tm_min).arg(ptm->tm_sec);
    QString status_info = tr("           cpu:%1%  mem:%2%").arg(cpuUsed*100,0,'f',2).arg(memUsed*100,0,'f',2);
    this->statusBar()->showMessage(cur_time+status_info);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    show_tab_info(index);
}

void MainWindow::on_pushButton_Refresh_clicked()
{
    show_tab_info(ui->tabWidget->currentIndex());
}

void MainWindow::on_pushButton_KillPro_clicked()
{
    //获得进程号
    int cur_row = ui->tableWidget_Process->currentRow();
    QString pid = ui->tableWidget_Process->item(cur_row,0)->text();
    QString p_name = ui->tableWidget_Process->item(cur_row,1)->text();

    int ret = QMessageBox::question(this, tr("kill"), tr("结束进程%1?").arg(p_name));
    if(ret == QMessageBox::Yes)
    {
        system("kill " + pid.toLatin1());//toLatin1:将QString转换成char *  
        QMessageBox::information(this, tr("kill"), QString::fromUtf8("该进程已结束!"), QMessageBox::Yes);
    }
    show_tab_info(ui->tabWidget->currentIndex());//刷新tab
}

void MainWindow::on_pushButton_Search_clicked()
{
    QString hint = ui->lineEdit_Search->text().trimmed();
    int row_count = ui->tableWidget_Process->rowCount();
    int row;
    QString pid,p_name;

    for(row=0;row<row_count;row++)
    {
        pid = ui->tableWidget_Process->item(row,0)->text();
        p_name = ui->tableWidget_Process->item(row,1)->text();
        if((pid == hint)||(p_name == hint))
        {
            break;
        }
    }

    if(row == row_count)
    {
        QMessageBox::warning(this,tr("查找进程"),tr("该进程不存在!"));
    }
    else
    {
        ui->tableWidget_Process->setCurrentCell(row, QItemSelectionModel::Select);
    }
}

//新建任务
void MainWindow::on_action_triggered()
{
    struct stat statbuf; //详细文件信息结构体

    QString new_thread_path = QFileDialog::getOpenFileName(this,tr("选择程序"),QCoreApplication::applicationDirPath());

    if(new_thread_path == "") return ;//如果用户选择取消

    lstat(new_thread_path.toLatin1(),&statbuf); //通过文件名，得到详细文件信息,用来判断当前选择的文件是不是可执行程序
    mode_t mode = statbuf.st_mode;
    if((mode&S_IXUSR)||(mode&S_IXGRP)||(mode&S_IXOTH))
    {
        if( fork() == 0)
        {
            execv(new_thread_path.toLatin1(),NULL);
        }
    }
    else
    {
        QMessageBox::critical(this,tr("严重错误"),tr("这不是一个可执行程序"));
    }
}


