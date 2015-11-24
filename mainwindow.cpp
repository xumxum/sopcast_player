#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QProcess"
#include "QTimer"
#include "QStandardItemModel"
#include "QStandardItem"
#include "QTableWidgetItem"
#include "QProgressBar"
#include "QTcpSocket"
#include "QThread"
#include "QMessageBox"


enum TableColumns
{
    colName,
    colSopLink,
    colStatus,
    colBuffer,
    colPlay,
    colDelete,

    col_TOTAL //leave it last so we know how many we have
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("SopCast Player");

    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(refreshStatus()));
    m_Timer->start(1000);

    ui->tableInfo->setColumnCount(6);
    ui->tableInfo->setHorizontalHeaderItem(colName, new QTableWidgetItem(QString("Name")));
    ui->tableInfo->setHorizontalHeaderItem(colSopLink, new QTableWidgetItem(QString("Address")));
    ui->tableInfo->setHorizontalHeaderItem(colStatus, new QTableWidgetItem(QString("Status")));
    ui->tableInfo->setHorizontalHeaderItem(colBuffer, new QTableWidgetItem(QString("Buffer")));
    ui->tableInfo->setHorizontalHeaderItem(colPlay, new QTableWidgetItem(QString("Play")));
    ui->tableInfo->setHorizontalHeaderItem(colDelete, new QTableWidgetItem(QString("Delete")));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString        program = "/work/test/sopcast_player/sopcast_player/sp.pl";
    QProcess* newSopcastProcess = new QProcess(this);

    newSopcastProcess->start(program);
    connect(newSopcastProcess, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
    connect(newSopcastProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(refreshStatus()));

    //m_SopCastProcesses.append( newSopcastProcess );
    PlayEntry entry;
    entry.m_Soplink = ui->text_soplink->text();
    entry.m_SopCastProces = newSopcastProcess;

    m_PlayEntries.append(entry);

    refreshStatus();

    //just to test the basic socket stuff...
    getStatuses(0);
}

void MainWindow::on_sopcast_exited(int exitStatus)
{
    //QString msg = QString("App finished exit code: ") + QString::number(exitStatus);
    //ui->label->setText(msg);

    //need to find it and remove it..by state?
    QVector<QProcess *>::iterator it;
#if 0
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
#endif

    refreshStatus();
}


void MainWindow::refreshStatus()
{
#if 0
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

    //ui->label->setText(statusAll);
    #endif

    //cleanup
    ui->tableInfo->setRowCount(0);

#if 1
    for(int i=0; i<m_PlayEntries.size(); i++)
    {
        int row = ui->tableInfo->rowCount();
        ui->tableInfo->insertRow(row);


        QTableWidgetItem *itemName= new QTableWidgetItem();
        QTableWidgetItem *itemSopLink = new QTableWidgetItem();
        QTableWidgetItem *itemStatus = new QTableWidgetItem();
        QTableWidgetItem *itemBuffer = new QTableWidgetItem();
        QTableWidgetItem *itemPlay = new QTableWidgetItem();
        QTableWidgetItem *itemDelete = new QTableWidgetItem();

        itemSopLink->setText(m_PlayEntries.at(i).m_Soplink);

        QProcess* processSopCast = m_PlayEntries.at(i).m_SopCastProces;
        if (processSopCast)
        {
            switch (processSopCast->state())
            {
            case QProcess::NotRunning:
                itemStatus->setText( QString("Not running") );
                break;
            case QProcess::Starting:
                itemStatus->setText( QString("Starting") );
                break;
            case QProcess::Running:
                itemStatus->setText( QString("Running") );
                break;
            default:
                break;
            }
        }

        QProgressBar *pgbar = new QProgressBar();
        pgbar->setValue(50);
        pgbar->setMaximum(100);

        //changing color, works
#if 0
        QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );border-bottom-right-radius: 5px;border-bottom-left-radius: 5px;border: .px solid black;}";
        pgbar->setStyleSheet(danger);
#endif

        ui->tableInfo->setItem(i, colName , itemName);
        ui->tableInfo->setItem(i, colSopLink , itemSopLink);
        ui->tableInfo->setItem(i, colStatus , itemStatus);
        //ui->tableInfo->setItem(i, colBuffer , itemBuffer);
        ui->tableInfo->setCellWidget(i, colBuffer , pgbar);

        ui->tableInfo->setItem(i, colPlay , itemPlay);
        ui->tableInfo->setItem(i, colDelete , itemDelete);

    }
#endif
}


void MainWindow::getStatuses(int connectionId)
{
    QTcpSocket sock;
    sock.connectToHost("127.0.0.1",12000);
    if (sock.waitForConnected(500))
    {
        //connected, send status\n
        sock.write("\nstatus\ns\n");
        sock.waitForBytesWritten();
        //QThread::msleep(50);

        sock.waitForReadyRead(50);
        QByteArray replyBytes = sock.readLine();
        QString reply(replyBytes);

        QStringList stats = reply.split(" ");
        if (stats.size()>0)
        {
            QMessageBox::warning(0,"Reply", stats[0]);
        }

        sock.disconnectFromHost();
    }
}
