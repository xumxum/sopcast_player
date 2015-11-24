#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QProcess"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("SopCast Player");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
#if 0
    ui->label->setText("Starting App");

    QString        program = "/work/test/sopcast_player/sopcast_player/sp.pl";
    QStringList    arguments;

    m_myProcess = new QProcess(this);

    m_myProcess->start(program, arguments);

    connect(m_myProcess, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
#endif

    QString        program = "/work/test/sopcast_player/sopcast_player/sp.pl";
    QProcess* newSopcastProcess = new QProcess(this);

    newSopcastProcess->start(program);
    connect(newSopcastProcess, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
    connect(newSopcastProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(refreshStatus()));

    m_SopCastProcesses.append( newSopcastProcess );

    refreshStatus();
}

void MainWindow::on_sopcast_exited(int exitStatus)
{
    //QString msg = QString("App finished exit code: ") + QString::number(exitStatus);
    //ui->label->setText(msg);

    //need to find it and remove it..by state?
    QVector<QProcess *>::iterator it;

    it = m_SopCastProcesses.begin();
    while( it != m_SopCastProcesses.end() )
    {
        if ((*it)->state() == QProcess::NotRunning)
        {
            //remove it + cleanup
            QProcess* stoppedSopClientProcess = *it;
            m_SopCastProcesses.erase(it);
            stoppedSopClientProcess->deleteLater(); //can't call delte from slot from signal from obj
            it = m_SopCastProcesses.begin();
        }
        else
            it++;
    }


    refreshStatus();
}


void MainWindow::refreshStatus()
{
    QString statusAll;

    for(int i=0; i<m_SopCastProcesses.size(); i++)
    {
        QString status = QString::number(i) + QString(" -> ") + QString::number(m_SopCastProcesses.at(i)->pid());
        switch (m_SopCastProcesses.at(i)->state())
        {
        status += QString("  ");
        case QProcess::NotRunning:
            status += QString("Not running");
            break;
        case QProcess::Starting:
            status += QString("Starting");
            break;
        case QProcess::Running:
            status += QString("Running");
            break;
        default:
            break;
        }

        status += "\n";
        statusAll += status;
    }

    ui->label->setText(statusAll);
}
