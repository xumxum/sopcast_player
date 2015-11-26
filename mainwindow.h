#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
class QProcess;
class QTimer;


struct PlayEntry
{
public:
    PlayEntry() : m_OutboundPort(0), m_BuffLevel(0) {}


    QString m_Name;
    QString m_Soplink;

    QProcess *m_SopCastProces;
    int m_OutboundPort;
    int m_BuffLevel;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_sopcast_exited(int exitStatus);
    void refreshStatus();

private:
    Ui::MainWindow *ui;

    //QVector<QProcess *> m_SopCastProcesses;

    QVector<PlayEntry> m_PlayEntries;

    QTimer* m_Timer;

    unsigned m_NextPortNr;

    void updateBuffStatus(int connectionId);
    void refreshBufferStatusses();


};

#endif // MAINWINDOW_H
