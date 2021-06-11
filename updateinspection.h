#ifndef UPDATEINSPECTION_H
#define UPDATEINSPECTION_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QThread>
#include <QTextBrowser>
#include <QDebug>
#include <QDateTime>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class UpdateInspection : public QThread
{
    Q_OBJECT
public:
    enum Net
    {
        UnKnow = 0,
        OK,
        NetworkConnectionError
    };
    void run() override;
    static void setDirPath(const QString &path) { dirPath = path; }
    UpdateInspection();
    ~UpdateInspection();

private:
    QString MODID = "";
    static QString dirPath;
    QTextBrowser *textBrowser;
    void UpdateInspectionStart();
    QNetworkAccessManager *QNA = nullptr;
    QNetworkReply* QNR = nullptr;
    bool waitNet = false;
    QDateTime fileTime;

signals:
    void haveInfo(QString);
    void isEnd();
    void startQNA(QString);
    void isNetOk();
    void haveTimeNet(QString);
};

#endif // UPDATEINSPECTION_H
