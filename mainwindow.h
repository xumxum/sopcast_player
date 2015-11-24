#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
class QProcess;
class QTimer;


struct PlayEntry
{
    QString m_Name;
    QString m_Soplink;

    QProcess *m_SopCastProces;
    int m_OutboundPort;
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
    //QProcess *m_myProcess;

    void getStatuses(int connectionId);


};

#endif // MAINWINDOW_H
