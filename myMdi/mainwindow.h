#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MdiChild;
class QMdiSubWindow;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    MdiChild *activeMdiChild();       //活动窗口
    QMdiSubWindow *findMdiChild(const QString &fileName);       //查找子窗口

private slots:
    void on_actionNew_triggered();
    void updataMenu();              //更新菜单
    MdiChild *creatMdiChild();      //创建子窗口
    void setActiveSubWindow(QWidget *window);       //设置活动窗口

    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;
    QAction *actionSeparator;       //间隔器
};

#endif // MAINWINDOW_H
