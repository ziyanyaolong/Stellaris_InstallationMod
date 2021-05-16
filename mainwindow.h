#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    bool copyDir(const QString &source, const QString &destination, bool override);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer * makerColor;
    int mode = 0;
    int modeDirCopy = 0;
private slots:
    void setModFolder();
    void setModIndex();
    void modeTest();
};
#endif // MAINWINDOW_H
