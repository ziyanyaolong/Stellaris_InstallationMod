#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QThread>

#include "updateinspection.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStringList errorList;
    QStringList httpsList;
    Ui::MainWindow *ui;
    QTimer * makerColor;

    int mode = 0;
    int modeDirCopy = 0;

    QStringList readCatalogue(const QString &path);

    bool deleteDir(const QString &path);
    bool copyDir(const QString &source, const QString &destination, bool override);

private slots:
    void setModFolder();
    void setModIndex();
    void modeTest();
    bool saveFilePath(const QString &a, const QString &b, const QString &c);
    QStringList readFilePath();
};
#endif // MAINWINDOW_H
