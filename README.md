# TaskManager
采用qt creator在unbuntu14.04环境下开发的任务管理器

# 1.实验设计  
## 1.1.整体设计
系统划分为如下几个模块：
1. **系统信息模块**，包含的功能有：
    1. 获取并显示主机名；
    2. 获取并显示系统启动的时间；
    3. 显示系统到目前为止持续运行的时间；
    4. 显示系统的版本号；
    5. 显示cpu的型号和主频大小。

2. **进程信息模块**，包含的功能有：
    1. 通过pid或者进程名查询一个进程，并显示该进程的详细信息，提供杀掉该进程的功能；
    2. 显示系统所有进程的一些信息，包括pid，ppid，占用内存大小，优先级等等。

3. **性能模块**，包含的功能有：
    1. cpu使用率的图形化显示；
    2. 内存和交换分区(swap)使用率的图形化显示。

4. **状态栏模块**，包含的功能有：
    1. 在状态栏显示当前时间；
    2. 在状态栏显示当前cpu使用率；
    3. 在状态栏显示当前内存使用情况。
5. **运行新进程模块**，包含的功能有：
    1. 用新进程运行一个其他程序。
6. **关机模块**，包含的功能有：
    1. 关机、重启、挂起功能。
## 1.2.详细设计
系统信息模块、进程信息模块和性能模块显示在一个Tab Widget控件中，用槽函数on_tabWidget_currentChanged响应页面变化事件，该函数中调用show_tab_info函数来根据当前的页面号调用不同的函数刷新界面；状态栏模块显示在状态栏中；运行新进程模块和关机模块显示在菜单栏中。
1. 系统信息模块  
该模块对应的函数是show_system_info，该函数中都是先读取相应文件中相应字段的信息，然后显示在相应的Label控件中。其中系统启动时间是由系统当前时间减去系统持续运行时间算出来的。
2. 进程信息模块  
该模块对应的函数是show_process_info，该函数通过读取/proc目录下所有数字文件夹下的stat文件中相应的字段内容获取所有进程的信息，然后将这些内容显示在TableWidget控件中。
查询功能对应的槽函数为on_pushButton_Search_clicked，当查询按钮按下时就会执行该函数。该函数首先读取查询按钮左边的LineEdit控件中输入的内容，然后将其与TableWidget中的前两列即pid和进程名对应的列中的内容进行对比，如果有相同的，则定位到该行，反之，用消息提示框提示用户查询的进程不存在。
刷新功能对应的槽函数是on_pushButton_Refresh_clicked，该函数就是调用了show_tab_info函数将当前tab页面刷新了。
结束进程功能对应的槽函数是on_pushButton_KillPro_clicked，该函数中首先获取当前选中的行对应的pid，然后用system函数使用系统命令“kill pid ”结束选定的进程。
3. 性能模块  
该模块对应的函数是show_capability，在该函数中主要是计算当前cpu、内存和交换分区的使用率，然后调用update函数触发paintEvent函数利用获得的数据绘制图像。由于图像需要动态刷新，所以利用定时器让show_capability函数每隔1s就被调用一次。
paintEvent函数中利用QPainter类在pixmap类上作图，用全局的数组pts来存储show_capability函数中计算出的cpu、内存和交换分区的使用率。而且需要动态调整pts数组中的数据：当曲线到达窗口最左端的时候就舍弃最前面的一个点；当窗口宽度变大时就扩大pts数组的长度；当窗口宽度变小时，先判断当前宽度能否容纳现有的数据，若能，则减小pts数组的长度，若不能，则舍弃前面不能绘制的点。为了让纵坐标变化时图形也不变形，pts数组中存储的是纵坐标的比例，当绘制图形时再乘以当前图像的高度。
4. 状态栏模块  
该模块需要动态刷新，所以用定时器每1s刷新一次，对应的槽函数为time_out_update_status_bar，当前时间是通过time函数获取的，cpu使用率和内存使用情况使用的是性能模块中的show_capability函数中计算出来的结果，最后调用StatusBar的showMessage函数显示信息。
5. 运行新进程模块  
该模块对应的函数是on_action_triggered，该函数中首先调用QFileDialog::getOpenFileName显示文件对话框，当用户选中文件后返回文件的路径。然后根据路径判断该文件是不是可执行文件，若是，则利用fork和execv函数运行该程序；若不是，则用的对话框提示用户。
6. 关机模块  
该模块对应的函数为on_action_3_triggered、on_action_4_triggered、on_action_5_triggered，分别对应关机、重启、挂起功能。关机命令是shutdown -h now或halt；重启命令是shutdown -r now或reboot；挂起命令是rtcwake -m mem -s time,该命令中可以让用户选择挂起多长时间。这三个功能都需要root权限。
# 2.实验结果
1. 系统信息模块    
    该模块的运行结果如图1所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%871.png)  
图 1

2. 进程信息模块  
    该模块的运行结果如图2所示。当输入CD，并点击查询后，焦点定位到该进程所在的行，如图2所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%872.png)   
图 2  
    然后点击结束进程按钮，系统弹出提示框，如图3所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%873.png)  
图 3  
选择yes，出现如图4所示对话框，表明已杀死该进程。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%874.png)  
图 4  
点击yes，发现进程总数减少了1个，如图5所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%875.png)  
图 5  
再次查询CD，系统提示该进程不存在，如图6所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%876.png)  
图 6  

3. 性能模块  
该模块的运行结果如图7所示，放大后的效果如图8所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%877.png)  
图 7  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%878.png)  
图 8  

4. 状态栏模块  
该模块运行结果如图9所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%879.png)  
图 9  

5. 运行新进程模块  
该模块在菜单上的运行结果如图10所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8710.png)  
图 10  
点击新建任务，出现如图11所示的对话框。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8711.png)  
图 11  
选中TaskManager,发现又有一个任务管理器运行，如图12所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8712.png)  
图 12  
若选择一个不可执行文件，出现如图13所示的错误提示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8713.png)  
图 13  

6. 关机模块  
该模块在菜单栏上的运行结果如图14所示。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8714.png)  
图 14  
为测试这三个功能，需要在终端先进入该进程所在的目录，然后输入sudo TaskManger。这时若选择关机，则系统关机；若选择重启，则系统重启；若选择挂起，则弹出如图15所示对话框，点击ok后系统会挂起用户选择的时间长度。  
![image](https://github.com/wangxinxinx/TaskManager/blob/master/images/%E5%9B%BE%E7%89%8715.png)  
图 15  
