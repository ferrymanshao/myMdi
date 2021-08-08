#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //创建间隔器动作并在其中设置间隔器
    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    //更新菜单
    updataMenu();
    //当有活动窗口时更新菜单
    connect(ui->mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow*)),this,SLOT(updataMenu()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    MdiChild *child = creatMdiChild();
    //多文档区域添加子窗口
    ui->mdiArea->addSubWindow(child);
    //新建文件
    child->newFile();
    child->show();
}

void MainWindow::updataMenu()
{
    //根据是否有活动窗口来设置各个动作是否可用
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSaceAs->setEnabled(hasMdiChild);
    ui->actionPaste->setEnabled(hasMdiChild);
    ui->actionClose->setEnabled(hasMdiChild);
    ui->actionCloseAll->setEnabled(hasMdiChild);
    ui->actionTile->setEnabled(hasMdiChild);
    ui->actionCascade->setEnabled(hasMdiChild);
    ui->actionNext->setEnabled(hasMdiChild);
    ui->actionPrevious->setEnabled(hasMdiChild);

    //设置间隔器是否显示
    actionSeparator->setVisible(hasMdiChild);
    //有活动窗口且有被选择的文本，剪切复制才可用
    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor().hasSelection());
    qDebug() << "hasSelection: " << hasSelection;
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    //有活动窗口且文档有撤销操作时撤销动作可用
    ui->actionUndo->setEnabled(activeMdiChild()
                               && activeMdiChild()->document()->isUndoAvailable());
    //有活动窗口且文档有恢复操作时恢复动作可用
    ui->actionRedo->setEnabled(activeMdiChild()
                               && activeMdiChild()->document()->isRedoAvailable());
}

MdiChild *MainWindow::creatMdiChild()
{
    //创建MdiChild部件
    MdiChild *child = new MdiChild;
    //向多文档区域添加子窗口，child为中心部件
    ui->mdiArea->addSubWindow(child);
    //根据QTextEdit类的是否可以复制信号设置剪切复制动作是否可用
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCopy,SLOT(setEnabled(bool)));
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCut,SLOT(setEnabled(bool)));
    //根据QTextDocument类的是否可以撤销恢复信号设置撤销恢复动作是否可用
    connect(child->document(),SIGNAL(undoAvailable(bool)),ui->actionUndo,SLOT(setEnabled(bool)));
    connect(child->document(),SIGNAL(redoAvailable(bool)),ui->actionRedo,SLOT(setEnabled(bool)));
    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    //如果传递了窗口部件，则将其设置为活动窗口
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

MdiChild *MainWindow::activeMdiChild()
{
    //如果有活动窗口，则将其中的中心部件转换为MdiChild类型，没有则直接返回0
    if(QMdiSubWindow *aCtiveSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<MdiChild *>(aCtiveSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    //利用foreach语句遍历子窗口列表，如果其文件路径和要查找的路径相同，则返回该窗口
    foreach(QMdiSubWindow *window,ui->mdiArea->subWindowList())
    {
        MdiChild *mdiChild = qobject_cast<MdiChild*>(window->widget());
        if(mdiChild->currentFile() == canonicalFilePath)
            return window;
    }

    return 0;
}

void MainWindow::on_actionOpen_triggered()
{
    //获取文件路径
    QString fileName = QFileDialog::getOpenFileName(this);
    //如果文件不为空，则查看该文件是否已经打开
    if(!fileName.isEmpty())
    {
        QMdiSubWindow *exiting = findMdiChild(fileName);
        //如果已经存在，则将对应的子窗口设置为活动窗口
        if(exiting)
        {
            ui->mdiArea->setActiveSubWindow(exiting);
            return;
        }
        //如果没有就新建窗口
        MdiChild *child = creatMdiChild();
        if(child->loadFile(fileName))
        {
            ui->statusBar->showMessage(QString::fromUtf8("打开文件成功"),2000);
            child->show();
        }
        else
        {
            child->close();
        }
    }
}




























