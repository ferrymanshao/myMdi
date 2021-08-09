#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MdiChild;
class QMdiSubWindow;
class QSignalMapper;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    MdiChild *activeMdiChild();       //活动窗口
    QMdiSubWindow *findMdiChild(const QString &fileName);       //查找子窗口
    void readSettings();              //读取窗口设置
    void writeSettings();             //写入窗口设置
    void initWindow();                //初始化窗口

protected:
    void closeEvent(QCloseEvent *event);    //关闭事件

private slots:
    void on_actionNew_triggered();
    void updataMenu();              //更新菜单
    void updataWindowMenu();        //更新窗口菜单
    MdiChild *creatMdiChild();      //创建子窗口
    void setActiveSubWindow(QWidget *window);       //设置活动窗口
    void showTextRowAndCol();           //显示文本的行号和列号
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionClose_triggered();
    void on_actionCloseAll_triggered();
    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    QAction *actionSeparator;       //间隔器
    QSignalMapper *windowMapper;    //信号映射器
};

#endif // MAINWINDOW_H
