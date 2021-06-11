#include "updateinspection.h"

QString UpdateInspection::dirPath = "";

UpdateInspection::UpdateInspection()
{
    QNA = new QNetworkAccessManager(this);
}

UpdateInspection::~UpdateInspection()
{
}

void UpdateInspection::run()
{
    UpdateInspectionStart();
}

void UpdateInspection::UpdateInspectionStart()
{

    connect(this, &UpdateInspection::isNetOk, this, [&](){
        waitNet = false;
    },Qt::QueuedConnection);

    connect(this, &UpdateInspection::startQNA, this, [&](QString https){
        waitNet = true;
        QNR = QNA->get(QNetworkRequest(QUrl(https)));
    },Qt::QueuedConnection);

    connect(QNA, &QNetworkAccessManager::finished, this, [&](){
        QNR->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QNR->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (QNR->error() == QNetworkReply::NoError)
        {
            QByteArray bytes = QNR->readAll();
            QString string = QString::fromUtf8(bytes);
            if(string.lastIndexOf("<div class=\"detailsStatRight\">") == -1)
            {
                emit haveInfo(MODID + ":" +QString("error") + "\n");
            }else
            {
                string = string.mid(string.lastIndexOf("<div class=\"detailsStatRight\">") + QString("<div class=\"detailsStatRight\">").size(), string.size());
                string = string.mid(0,string.indexOf("</div>"));
                if(string.indexOf("年") == -1)
                {
                    QDate currentdate = QDate::currentDate();
                    QString str1 = currentdate.toString("yyyy");
                    string = str1 + "年" + string;
                }
                QString string2 = string;
                string2.replace("年","-");
                string2.replace("月","-");
                string2.replace("日","");
                string2.replace("上午"," ");
                string2.replace("下午"," ");
                string2 += ":00";
                QDateTime time1 = QDateTime::fromString(string2, "yyyy-M-d h:m:s");
                QDateTime time2 = fileTime;
                uint stime = time1.toTime_t();
                uint etime = time2.toTime_t();

                int tRet = stime - etime;

                if(tRet > 0)
                {
                    emit haveInfo(MODID + ":" +QString(string) + "——" + "haveNew!" + "\n");
                }
            }
        }
        else
        {
            qDebug()<< QNR->errorString();
        }

        QNR->deleteLater();
        emit isNetOk();
    },Qt::QueuedConnection);

    QDir dir(dirPath);
    QFileInfoList info_list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> i(info_list);

    //循环检测直到结束
    while (i.hasNext())
    {
        QFileInfo info = i.next();

        QDir dir2(info.filePath());
        QFileInfoList info_list2 = dir2.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QListIterator<QFileInfo> j(info_list2);
        while (j.hasNext())
        {
            QFileInfo info2 = j.next();
            if(info2.fileName() != "descriptor.mod")
            {
                QStringList SL;
                fileTime = info2.lastModified().toLocalTime();
                QFile file(info2.filePath());
                QTextStream in(&file);
                file.setPermissions(QFile::ReadUser | QFile::WriteUser);

                //打开descriptor.mod文件
                if (file.open(QIODevice::ReadWrite))
                {
                    while(!file.atEnd())
                    {
                        SL.push_back(file.readLine());
                    }
                }
                file.close();
                for(int i = 0; i < SL.size(); i++)
                {
                    if(SL[i].indexOf("remote_file_id=") != -1)
                    {
                        MODID = SL[i].mid(SL[i].indexOf("\"") + QString("\"").size());
                        MODID = MODID.mid(0, MODID.lastIndexOf("\""));
                        emit startQNA(QString("https://steamcommunity.com/sharedfiles/filedetails/?id=") + MODID);
                        break;
                    }
                }
                do{
                    msleep(100);
                }
                while (waitNet);
            }
        }
    }
    while (waitNet)
    {
        msleep(100);
    }
    qDebug() << "End!";
    exit(0);
//    qDebug() <<
//    remote_file_id=
}
