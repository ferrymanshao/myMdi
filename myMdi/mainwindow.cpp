#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
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

    //创建信号映射器
    windowMapper = new QSignalMapper(this);
    //映射器重新发射信号，根据信号设置活动窗口
    connect(windowMapper,SIGNAL(mapped(QWidget*)),this,SLOT(setActiveSubWindow(QWidget*)));

    //更新窗口菜单，并且设置当窗口菜单将要显示的时候更新窗口菜单
    updataWindowMenu();
    connect(ui->menuW,SIGNAL(aboutToShow()),this,SLOT(updataWindowMenu()));

    readSettings();

    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    MdiChild *child = creatMdiChild();
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

void MainWindow::updataWindowMenu()
{
    //先清空菜单，然后再添加各个菜单动作
    ui->menuW->clear();
    ui->menuW->addAction(ui->actionClose);
    ui->menuW->addAction(ui->actionCloseAll);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionTile);
    ui->menuW->addAction(ui->actionCascade);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionNext);
    ui->menuW->addAction(ui->actionPrevious);
    ui->menuW->addAction(actionSeparator);

    //如果有活动窗口，则显示间隔器
    QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());

    //遍历各个子窗口
    for(int i = 0; i < windows.size(); ++i)
    {
        MdiChild *child = qobject_cast<MdiChild*>(windows.at(i)->widget());
        QString text;
        //如果窗口数小于9，则设置编号为快捷键
        if(i < 9)
        {
            text = QString::fromUtf8("&%1 %2").arg(i+1)
                    .arg(child->userFriendlyCurrentFile());
        }
        else
        {
            text = QString::fromUtf8("%1 %2").arg(i+1)
                    .arg(child->userFriendlyCurrentFile());
        }
        //添加动作到菜单，设置动作可以选择
        QAction *action = ui->menuW->addAction(text);
        action->setCheckable(true);
        //设置当前活动窗口动作为选中状态
        action->setChecked(child == activeMdiChild());
        //关联动作的触发信号到信号映射器的map()槽，这个槽会发射mappde()信号
        connect(action,SIGNAL(triggered()),windowMapper,SLOT(map()));
        //将动作与相应的窗口部件进行映射
        //在发射mapped()信号时就会以这个窗口部件为参数
        windowMapper->setMapping(action,windows.at(i));
    }
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
    //每当编辑器中的光标位置改变，就重新显示行号和列号
    connect(child,SIGNAL(cursorPositionChanged()),this,SLOT(showTextRowAndCol()));
    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    //如果传递了窗口部件，则将其设置为活动窗口
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

void MainWindow::showTextRowAndCol()
{
    if(activeMdiChild())
    {
        //因为获取的行号和列号都是从0开始的，所以我们这里进行了加1
        int rowNum = activeMdiChild()->textCursor().blockNumber() + 1;
        int colNum = activeMdiChild()->textCursor().columnNumber() + 1;
        ui->statusBar->showMessage(QString::fromUtf8("%1行 %2列").arg(rowNum).arg(colNum));
    }
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

void MainWindow::readSettings()
{
    QSettings setting("ferryman","myMdi");
    QPoint pos = setting.value("pos",QPoint(200,200)).toPoint();
    QSize size = setting.value("size",QSize(400,400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings setting("ferryman","myMdi");
    //写入位置信息和大小信息
    setting.setValue("pso",pos());
    setting.setValue("size",size());
}

void MainWindow::initWindow()
{
    setWindowTitle(QString::fromUtf8("多文档编辑器"));
    //在工具栏上右击时，可以关闭工具栏
    ui->mainToolBar->setWindowTitle(QString::fromUtf8("工具栏"));
    //当多文档区域的内容超出可视区域后，出现滚动条
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->statusBar->showMessage(QString::fromUtf8("欢迎使用多文档编辑器"));
    QLabel *label = new QLabel(this);
    label->setFrameStyle(QFrame::Box | QFrame::Sunken);
    label->setText("<a href=\"https://www.baidu.com/s?wd=%E4%BD%A0%E6%98%AF%E7%8C%AA%E5%90%97&rsv_spt=1&rsv_iqid=0x96d29ee000012f46&issp=1&f=8&rsv_bp=1&rsv_idx=2&ie=utf-8&tn=27082910_5_hao_pg&rsv_enter=1&rsv_dl=tb&rsv_sug3=13&rsv_sug1=5&rsv_sug7=100&rsv_sug2=0&rsv_btype=i&inputT=3424&rsv_sug4=3424\">bangzhu</a>");
    //标签为富文本
    label->setTextFormat(Qt::RichText);
    label->setOpenExternalLinks(true);
    ui->statusBar->addPermanentWidget(label);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //先执行多文档区域的关闭操作
    ui->mdiArea->closeAllSubWindows();
    //如果还有窗口没有关闭，则忽略该事件
    if(ui->mdiArea->currentSubWindow())
    {
        event->ignore();
    }
    else
    {
        //在关闭前写入窗口设置
        writeSettings();
        event->accept();
    }
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

void MainWindow::on_actionSave_triggered()
{
    if(activeMdiChild() && activeMdiChild()->save())
        ui->statusBar->showMessage(QString::fromUtf8("文件保存成功"),2000);
}

void MainWindow::on_actionUndo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->redo();
}

void MainWindow::on_actionCut_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->paste();
}

void MainWindow::on_actionClose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_actionCloseAll_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actionExit_triggered()
{
    //等价于QApplication::closeAllWindows()
    qApp->closeAllWindows();
}
