#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    MdiChild *child = new MdiChild;
    //多文档区域添加子窗口
    ui->mdiArea->addSubWindow(child);
    //新建文件
    child->newFile();
    child->show();
}
