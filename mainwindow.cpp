#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QTextCodec>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QPalette>
#include <QSslSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    makerColor = new QTimer;
    makerColor->start(10);

    QStringList temp = readFilePath();
    if(temp.isEmpty())
    {
        ui->lineEdit->setText("");
        ui->lineEdit_2->setText("");
        ui->lineEdit_3->setText("");
    }else
    {
        for (QStringList::iterator i = temp.begin(); i != temp.end() ;i++)
        {
            *i = i->trimmed();
        }
        ui->lineEdit->setText(temp[0]);
        ui->lineEdit_2->setText(temp[1]);
        ui->lineEdit_3->setText(temp[2]);
    }

    connect(makerColor, &QTimer::timeout, this, [=]
            {
                static bool peTemp = false;
                static int peColorTemp = -255;
                QPalette pe;
                if (!peTemp)
                {
                    pe.setColor(QPalette::WindowText, peColorTemp);
                    ui->label_4->setPalette(pe);
                    if (peColorTemp >= 255)
                    {
                        peTemp = true;
                    }
                    peColorTemp++;
                }
                else if (peTemp)
                {
                    pe.setColor(QPalette::WindowText, peColorTemp);
                    ui->label_4->setPalette(pe);
                    if (peColorTemp < -255)
                    {
                        peTemp = false;
                    }
                    peColorTemp--;
                }
            });

    //(...)按钮选择路径
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=]()
            {
                ui->lineEdit_3->setText(QFileDialog::getExistingDirectory(this, "选择文件夹"));
                saveFilePath(ui->lineEdit->text(),ui->lineEdit_2->text(),ui->lineEdit_3->text());
            });

    connect(ui->pushButton_3, &QPushButton::clicked, this, [=]()
            {
                ui->lineEdit_2->setText(QFileDialog::getExistingDirectory(this, "选择文件夹"));
                saveFilePath(ui->lineEdit->text(),ui->lineEdit_2->text(),ui->lineEdit_3->text());
            });

    connect(ui->pushButton_4, &QPushButton::clicked, this, [=]()
            {
                ui->lineEdit->setText(QFileDialog::getExistingDirectory(this, "选择文件夹"));
                saveFilePath(ui->lineEdit->text(),ui->lineEdit_2->text(),ui->lineEdit_3->text());
            });

    connect(ui->pushButton_7, &QPushButton::clicked, this, [=]()
            {
                ui->textBrowser->insertPlainText("开始更新检测\n");
                UpdateInspection::setDirPath(ui->lineEdit_3->text());
                ui->textBrowser->moveCursor(QTextCursor::End);
                UpdateInspection *updateInspection = new UpdateInspection();

                connect(updateInspection, &UpdateInspection::finished, this, [&](){
                    ui->textBrowser->insertPlainText("检测完成\n");
                    if(httpsList.size() != 0)
                    {
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText("需要更新的mod的地址如下\n");
                        for(int i = 0; i < httpsList.size(); i++)
                        {
                            ui->textBrowser->insertPlainText(QString("https://steamcommunity.com/sharedfiles/filedetails/?id=") + httpsList[i] + "\n");
                        }
                    }else
                    {
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText("所有mod均已最新\n");
                    }

                    if(errorList.size() != 0)
                    {
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText("检测到error的地址如下\n");
                        for(int i = 0; i < errorList.size(); i++)
                        {
                            ui->textBrowser->insertPlainText(QString("https://steamcommunity.com/sharedfiles/filedetails/?id=") + errorList[i] + "\n");
                        }
                    }
                });
                connect(updateInspection, &UpdateInspection::haveInfo, this, [=](QString info){
                    if(info.indexOf("error") != -1)
                    {
                        errorList.push_back(info.mid(0,info.indexOf(":")));
                    }else
                    {
                        httpsList.push_back(info.mid(0,info.indexOf(":")));
                    }
                    ui->textBrowser->insertPlainText(info);
                });
                updateInspection->start();
            });

    connect(ui->pushButton_5, &QPushButton::clicked, this, [=]()
            {
                if (mode == 0)
                {
                    ui->pushButton_5->setText("当前模式：索引模式");
                    ui->label->setText("mod索引地址");
                    ui->pushButton_6->hide();
                    mode = 1;
                }
                else if (mode == 1)
                {
                    ui->pushButton_5->setText("当前模式：文件夹模式");
                    ui->label->setText("mod文件夹地址");
                    ui->pushButton_6->show();
                    mode = 0;
                }
            });

    connect(ui->pushButton_6, &QPushButton::clicked, this, [=]()
            {
                if (modeDirCopy == 0)
                {
                    ui->pushButton_6->setText("文件夹处理方式：不复制");
                    modeDirCopy = 1;
                }
                else if (modeDirCopy == 1)
                {
                    ui->pushButton_6->setText("文件夹处理方式：直接复制到地址");
                    modeDirCopy = 0;
                }
            });

    //启动开关
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::modeTest);

}

void MainWindow::modeTest()
{
    if (mode == 0)
    {
        setModFolder();
    }
    else if (mode == 1)
    {
        setModIndex();
    }
    else
    {
        mode = 1;
        emit ui->pushButton_5->clicked();
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("模式错误，已重置");
    }
}

void MainWindow::setModFolder()
{
    //存储路径
    QString modFile = ui->lineEdit->text();
    QString modIndex = ui->lineEdit_2->text();
    QString modOntology = ui->lineEdit_3->text();

    //替换mod文件夹路径的\为/
    for (int i = 0; i < modOntology.size(); i++)
    {
        if (modOntology[i] == '\\')
        {
            modOntology[i] = '/';
        }
    }
    //项目计数（总、成功、失败）
    unsigned int number = 0;
    unsigned int numberC = 0;
    unsigned int numberS = 0;

    //检测地址是否为空，为空则提示，并且直接退出函数
    if (modIndex.isEmpty())
    {
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("mod索引文件放置地址不能为空\n");
        return;
    }

    if (modOntology.isEmpty())
    {
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("mod本体放置地址不能为空\n");
        return;
    }

    if (modFile.isEmpty())
    {
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("原mod文件夹地址不能为空\n");
        return;
    }

    //读取根文件夹下的各个mod文件夹
    QDir dir(modFile);
    QFileInfoList info_list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> i(info_list);

    //循环检测直到结束
    while (i.hasNext())
    {
        QFileInfo info = i.next();
        number++;
        QString fileName = info.fileName();

        //读取各mod文件夹的所有文件夹和文件
        QDir dir2(info.filePath());
        QFileInfoList info_list2 = dir2.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QListIterator<QFileInfo> j(info_list2);
        bool dirTest = false;
        //循环检测直到结束
        while (j.hasNext())
        {
            QFileInfo info2 = j.next();
            //读取mod文件夹下是否存在"descriptor.mod"文件
            if (info2.fileName().indexOf("descriptor.mod") != -1)
            {
                dirTest = true;
                QString modNumberTemp = info.fileName();
                QString modNumber = modNumberTemp.mid(0, modNumberTemp.indexOf("_"));
                QFile modFile(info2.filePath());
                QTextStream in(&modFile);
                modFile.setPermissions(QFile::ReadUser | QFile::WriteUser);

                //打开descriptor.mod文件
                if (!modFile.open(QIODevice::ReadWrite))
                {
                    //失败就退出，并增加计数和发出提示
                    numberS++;
                    ui->textBrowser->moveCursor(QTextCursor::End);
                    ui->textBrowser->insertPlainText(modNumber + " NO\n");
                    continue;
                }
                //开始读取descriptor.mod文件中的内容，并加入到mtemp2中
                QString mtemp0 = " ";
                QList<QString> mtemp2;
                bool mtemp1 = false;
                while (!in.atEnd())
                {
                    mtemp0 = in.readLine();
                    mtemp2.append(mtemp0);
                    if (mtemp0.indexOf("path="))
                    {
                        //如果检测到descriptor.mod文件里已经含有mod放置路径的指示了，那么mtemp1设置为true
                        mtemp1 = true;
                    }
                }
                //如果mtemp1为true，那么说明descriptor.mod文件中已经含有mod放置路径的指示，否则，descriptor.mod文件夹中没有mod放置路径指示
                if (mtemp1)
                {
                    bool circulation = true;
                    while (circulation)
                    {
                        circulation = false;
                        //把mtemp2中的已有的mod放置路径（path="XXX"）的内容删去
                        QList<QString>::iterator iii = mtemp2.begin();
                        for (int ccc = 0; iii != mtemp2.end(); iii++)
                        {
                            if (mtemp2[ccc].indexOf("path=") != -1)
                            {
                                circulation = true;
                                mtemp2.erase(iii);
                                break;
                            }
                            ccc++;
                        }
                    }
                    //在mtemp2最后面添加mod放置路径
                    mtemp2.append(QString("path=\"" + modOntology + "/" + info.fileName() + "\""));
                    QFile modFile2(info2.path() + "/" + modNumber + ".mod");
                    //把mtemp2的内容写入到新文件中去
                    if (!modFile2.open(QIODevice::WriteOnly))
                    {
                        //失败就退出，并增加计数和发出提示
                        numberS++;
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText(modNumber + " NO\n");
                        continue;
                    }
                    foreach (QString mtemp4, mtemp2)
                    {
                        modFile2.write((mtemp4 + "\n").toUtf8());
                    }
                    //关闭所有文件（好习惯Good！）
                    modFile2.close();
                    modFile.close();
                }
                else
                {
                    //在文件最后写入mod放置路径
                    modFile.write(QString("path=\"" + modOntology + "/" + info.fileName() + "\"").toUtf8());
                    //关闭文件（好习惯Good！）
                    modFile.close();
                    if (modFile.fileName() != info2.path() + "/" + modNumber + ".mod")
                    {
                        //重命名文件为mod文件夹的前面的一串数字（其实起任意名称也没事，只是为了以后mod出错找文件方便）
                        modFile.rename(info2.path() + "/" + modNumber + ".mod");
                    }
                }
                //拷贝.mod文件
                QString oldFilePath = info2.path() + "/" + modNumber + ".mod";
                QString newFilePath = modIndex + "/" + modNumber + ".mod";

                QFile tempFile(newFilePath);

                if(tempFile.size() != 0)
                {
                    if(tempFile.remove())
                    {
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText(modNumber + ".mod" + QString("是重复文件，已替换") + "\n");
                    }else
                    {
                        ui->textBrowser->moveCursor(QTextCursor::End);
                        ui->textBrowser->insertPlainText(modNumber + ".mod" + QString("是重复文件，出现错误，无法替换") + "\n");
                    }
                }
                if (QFile::copy(oldFilePath, newFilePath))
                {
                    if (modeDirCopy == 0)
                    {
                        QString oldDirPath = info2.path();
                        QString newDirPath = ui->lineEdit_3->text() + "/" + info.fileName();

                        QDir tempDir(newDirPath);

                        if(!tempDir.isEmpty())
                        {
                            if(deleteDir(newDirPath))
                            {
                                ui->textBrowser->moveCursor(QTextCursor::End);
                                ui->textBrowser->insertPlainText(info.fileName() + QString("是重复文件夹，已替换") + "\n");
                            }else
                            {
                                ui->textBrowser->moveCursor(QTextCursor::End);
                                ui->textBrowser->insertPlainText(info.fileName() + QString("是重复文件夹，出现错误，无法替换") + "\n");
                            }
                        }

                        //如果成功则进一步拷贝mod本体文件夹
                        if (copyDir(oldDirPath, newDirPath, false))
                        {
                            //两个（mod索引和mod本体文件夹）拷贝都成功后计数加一并提示
                            numberC++;
                            ui->textBrowser->moveCursor(QTextCursor::End);
                            ui->textBrowser->insertPlainText(modNumber + " 文件夹OK\n");
                        }
                        else
                        {
                            //失败就退出，并增加计数和发出提示，但是mod索引文件拷贝已成功，可以到你自己设置的mod索引放置位置查看
                            numberS++;
                            ui->textBrowser->moveCursor(QTextCursor::End);
                            ui->textBrowser->insertPlainText(modNumber + " 文件夹NO\n");
                            continue;
                        }
                    }
                    else if (modeDirCopy == 1)
                    {
                        numberC++;
                    }
                    else
                    {
                        exit(1);
                    }

                    ui->textBrowser->moveCursor(QTextCursor::End);
                    ui->textBrowser->insertPlainText(modNumber + " 索引文件OK\n");
                }
                else
                {
                    //失败就退出，并增加计数和发出提示（此时mod索引文件和mod本体文件拷贝都失败）
                    numberS++;
                    ui->textBrowser->moveCursor(QTextCursor::End);
                    ui->textBrowser->insertPlainText(modNumber + " 索引文件NO\n");
                    continue;
                }
            }
        }
        if (!dirTest)
        {
            ui->textBrowser->moveCursor(QTextCursor::End);
            ui->textBrowser->insertPlainText(info.fileName() + " 此文件夹为空\n");
        }
    }

    //完成发出提示
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText(QString("项目数目：") + QString::number(number) + "\n");
    ui->textBrowser->insertPlainText(QString("成功项目：") + QString::number(numberC) + "\n");
    ui->textBrowser->insertPlainText(QString("失败项目：") + QString::number(numberS) + "\n");
    ui->textBrowser->moveCursor(QTextCursor::End);

    //如果成功数量不等于项目数量且失败数量为零，则有mod文件夹下有压缩包文件，需要解压后才能导出（可以使用搜索功能来搜索压缩文件）（自动解压缩待写）
    if ((number != numberC) && (numberS == 0))
    {
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("请检查各mod文件夹下是否有压缩包文件，请解压后再重试！（其他文件已操作成功）\n");
    }
}

void MainWindow::setModIndex()
{
    unsigned int number = 0;
    unsigned int numberC = 0;
    unsigned int numberS = 0;

    //存储路径
    QString modFile = ui->lineEdit->text();
    QString modIndex = ui->lineEdit_2->text();
    QString modOntology = ui->lineEdit_3->text();

    QStringList mods;

    QDir dir(ui->lineEdit->text());
    QFileInfoList info_list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> i(info_list);

    //循环检测直到结束
    while (i.hasNext())
    {
        QFileInfo info = i.next();
        //读取mod文件夹下是否存在"descriptor.mod"文件
        if (info.fileName().indexOf(".mod") != -1)
        {
            QString modNumberTemp = info.fileName();
            QString modNumber = modNumberTemp.mid(0, modNumberTemp.indexOf("_"));
            QFile modFile(info.filePath());
            QTextStream in(&modFile);
            modFile.setPermissions(QFile::ReadUser | QFile::WriteUser);

            //打开descriptor.mod文件
            if (!modFile.open(QIODevice::ReadWrite))
            {
                //失败就退出，并增加计数和发出提示
                numberS++;
                ui->textBrowser->moveCursor(QTextCursor::End);
                ui->textBrowser->insertPlainText(modNumber + " NO\n");
                continue;
            }
            //开始读取descriptor.mod文件中的内容，并加入到mtemp2中
            QString mtemp0 = " ";
            QList<QString> mtemp2;
            bool mtemp1 = false;
            while (!in.atEnd())
            {
                mtemp0 = in.readLine();
                mtemp2.append(mtemp0);
                if (mtemp0.indexOf("path=") != -1)
                {
                    //如果检测到descriptor.mod文件里已经含有mod放置路径的指示了，那么mtemp1设置为true
                    mtemp1 = true;
                }
            }
            //如果mtemp1为true，那么说明descriptor.mod文件中已经含有mod放置路径的指示，否则，descriptor.mod文件夹中没有mod放置路径指示
            if (mtemp1)
            {
                //把mtemp2中的已有的mod放置路径（path="XXX"）的内容删去
                QList<QString>::iterator iii = mtemp2.begin();
                bool temp = true;
                while (temp)
                {
                    temp = false;
                    for (int ccc = 0; iii != mtemp2.end(); iii++)
                    {
                        if (mtemp2[ccc].indexOf("path=") != -1)
                        {
                            temp = true;
                            mtemp2.erase(iii);
                            break;
                        }
                        ccc++;
                    }
                }
                //在mtemp2最后面添加mod放置路径
                if (info.fileName().indexOf(".mod") != -1)
                {
                    QString temp = info.fileName();
                    temp = temp.mid(0, temp.indexOf(".mod"));
                    modNumber = temp;
                }
                QString modName = "";
                QList<QString>::iterator jjj = mods.begin();
                for (int ccc = 0; jjj != mods.end(); jjj++)
                {
                    if (mods[ccc].indexOf(modNumber) != -1)
                    {
                        modName = mods[ccc];
                        mods.erase(jjj);
                        break;
                    }
                    ccc++;
                }
                mtemp2.append(QString("path=\"" + modOntology + "/" + modName + "\""));
                QFile modFile2(info.path() + "/" + modNumber + ".mod");
                //把mtemp2的内容写入到新文件中去
                if (!modFile2.open(QIODevice::WriteOnly))
                {
                    //失败就退出，并增加计数和发出提示
                    numberS++;
                    ui->textBrowser->moveCursor(QTextCursor::End);
                    ui->textBrowser->insertPlainText(modNumber + " NO\n");
                    continue;
                }
                foreach (QString mtemp4, mtemp2)
                {
                    modFile2.write((mtemp4 + "\n").toUtf8());
                }
                //关闭所有文件（好习惯Good！）
                modFile2.close();
                modFile.close();
            }
            else
            {
                //在文件最后写入mod放置路径
                modFile.write(QString("path=\"" + modOntology + "/" + info.fileName() + "\"").toUtf8());
                //关闭文件（好习惯Good！）
                modFile.close();
                if (modFile.fileName() != info.path() + "/" + modNumber + ".mod")
                {
                    //重命名文件为mod文件夹的前面的一串数字（其实起任意名称也没事，只是为了以后mod出错找文件方便）
                    if (info.fileName().indexOf(".mod") != -1)
                    {
                        QString temp = info.fileName();
                        temp = temp.mid(0, temp.indexOf(".mod"));
                        modNumber = temp;
                    }
                    modFile.rename(info.path() + "/" + modNumber + ".mod");
                }
            }

            //拷贝.mod文件
            if (QFile::copy(QString(info.path() + "/" + modNumber + ".mod"), QString(modIndex + "/" + modNumber + ".mod")))
            {
                numberC++;
                ui->textBrowser->moveCursor(QTextCursor::End);
                ui->textBrowser->insertPlainText(modNumber + " 索引文件OK\n");
            }
            else
            {
                //失败就退出，并增加计数和发出提示（此时mod索引文件和mod本体文件拷贝都失败）
                numberS++;
                ui->textBrowser->moveCursor(QTextCursor::End);
                ui->textBrowser->insertPlainText(modNumber + " 索引文件NO\n");
                continue;
            }
        }
    }

    //完成发出提示
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText(QString("项目数目：") + QString::number(number) + "\n");
    ui->textBrowser->insertPlainText(QString("成功项目：") + QString::number(numberC) + "\n");
    ui->textBrowser->insertPlainText(QString("失败项目：") + QString::number(numberS) + "\n");

    //如果成功数量不等于项目数量且失败数量为零，则有mod文件夹下有压缩包文件，需要解压后才能导出（可以使用搜索功能来搜索压缩文件）（自动解压缩待写）
    if ((number != numberC) && (numberS == 0))
    {
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->insertPlainText("请检查各mod文件夹下是否有压缩包文件，请解压后再重试！（其他文件已操作成功）\n");
    }
}

bool MainWindow::saveFilePath(const QString &a, const QString &b, const QString &c)
{
    QFile file(QDir::currentPath() + "/" + "SavePath");
    if(!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    file.write(QString(a + "\n").toLocal8Bit());
    file.write(QString(b + "\n").toLocal8Bit());
    file.write(QString(c + "\n").toLocal8Bit());
    return true;
}

QStringList MainWindow::readFilePath()
{
    QStringList temp;
    QFile file(QDir::currentPath() + "/" + "SavePath");
    if(!file.open(QIODevice::ReadOnly))
    {
        return temp;
    }
    for (int i = 0; i < 3; i ++)
    {
        temp.push_back(file.readLine());
    }
    return temp;
}


//文件夹拷贝函数（借鉴于网上代码）
bool MainWindow::copyDir(const QString &source, const QString &destination, bool override)
{
    QDir sourceDir(source);
    QDir targetDir(destination);
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    /**< 当为目录时，递归的进行copy */
            if(!copyDir(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()),
                override))
                return false;
        }
        else{            /**< 当允许覆盖操作时，将旧文件进行删除操作 */
            if(override && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            /// 进行文件copy
            if(!QFile::copy(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()))){
                    return false;
            }
        }
    }
    return true;
}

//文件夹删除函数（借鉴于网上代码）
bool MainWindow::deleteDir(const QString &path)
{
    if (path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if(!dir.exists())
    {
        return true;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    { //遍历文件信息
        if (file.isFile()){ // 是文件，删除
            file.dir().remove(file.fileName());
        }else{ // 递归删除
            deleteDir(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());
}

QStringList MainWindow::readCatalogue(const QString &path)
{
    QStringList temp;
    QDir dir(path);
    QFileInfoList info_list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> i(info_list);

    //循环检测直到结束
    while (i.hasNext())
    {
        QFileInfo info = i.next();
        temp.append(info.fileName());
    }
    return temp;
}

//MainWindow析构函数
MainWindow::~MainWindow()
{
    delete ui;
}
