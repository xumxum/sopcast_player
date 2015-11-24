#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
class QProcess;

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

    QVector<QProcess *> m_SopCastProcesses;
    //QProcess *m_myProcess;




};

#endif // MAINWINDOW_H
