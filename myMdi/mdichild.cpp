#include "mdichild.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPushButton>


MdiChild::MdiChild(QWidget *parent)
{
    //设置在子窗口关闭时销毁这个类的对象
    setAttribute(Qt::WA_DeleteOnClose);
    //初始isUntitled为true
    isUntitled = true;
}

void MdiChild::newFile()
{
    //设置窗口编号，因为编号一直被保存，所以需要使用静态变量
    static int sequenceNumber = 1;
    //新建文档没有被保存
    isUntitled = true;
    //将当前文件命名为未命名文档加编号，编号先使用再加1
    curFile = QString::fromUtf8("未命名文档%1.txt").arg(sequenceNumber++);
    //设置标题
    setWindowTitle(curFile + "[*]" + QString::fromUtf8("- 多文档编辑器"));
    //文档更改时发射信号
    connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
}

bool MdiChild::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,tr.fromUtf8("多文档编辑器"),
                             tr.fromUtf8("无法读取文件%1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }
    
    //新建文本流对象
    QTextStream in(&file);
    //设置鼠标状态为等待状态
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //读取文件全部内容，并添加到编辑器中
    setPlainText(in.readAll());
    //回复鼠标状态
    QApplication::restoreOverrideCursor();
    //设置当前文件
    setCurrentFile(fileName);
    connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
    return true;
}

bool MdiChild::save()
{
    //如果文件未被保存过，则执行另存为操作，否则直接保存
    if(!isUntitled)
    {
        return saveAs();
    }
    else
    {
        saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr.fromUtf8("另存为"),curFile);
    if(fileName.isEmpty())
        return false;
    else
        return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,tr.fromUtf8("多文档编辑器"),
                             tr.fromUtf8("无法写入文件%1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return QFileInfo(curFile).fileName();
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if(maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MdiChild::documentWasModified()
{
    //根据文档的isModified()函数的返回值，判断编辑器内容是否被更改了
    //如果被更改了，就要在设置了[*]号的地方显示“*”号，这里会在窗口标题中显示
    setWindowModified(document()->isModified());
}

void MdiChild::setCurrentFile(const QString &fileName)
{
    //canonicalFilePath()可以出去路径中的符号链接，"."和".."等符号
    curFile = QFileInfo(fileName).canonicalFilePath();
    
    //文件已经被保存过了
    isUntitled  = false;
    //文档没有被更改过
    document()->setModified(false);
    //窗口不显示被更改标志
    setWindowModified(false);
    //设置窗口标题，userFriendlyCurrentFile()返回文件名
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}





































