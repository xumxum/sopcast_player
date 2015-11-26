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
#include "QIcon"

enum TableColumns
{
    colName,
    colSopLink,
    colStatus,
    colBuffer,
    colPortNr,
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

    m_NextPortNr = 9500;

    ui->tableInfo->setColumnCount(col_TOTAL);
    ui->tableInfo->setHorizontalHeaderItem(colName, new QTableWidgetItem(QString("Name")));
    ui->tableInfo->setHorizontalHeaderItem(colSopLink, new QTableWidgetItem(QString("Address")));
    ui->tableInfo->setHorizontalHeaderItem(colStatus, new QTableWidgetItem(QString("Status")));
    ui->tableInfo->setHorizontalHeaderItem(colBuffer, new QTableWidgetItem(QString("Buffer")));
    ui->tableInfo->setHorizontalHeaderItem(colPortNr, new QTableWidgetItem(QString("Port")));
    ui->tableInfo->setHorizontalHeaderItem(colPlay, new QTableWidgetItem(QString("Play")));
    ui->tableInfo->setHorizontalHeaderItem(colDelete, new QTableWidgetItem(QString("Delete")));

    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(refreshStatus()));
    m_Timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //QString        program = "/work/test/sopcast_player/sopcast_player/sp.pl";
    QString program = QString("sp-sc %1 %2 %3").arg(ui->text_soplink->text(),
                          QString::number(m_NextPortNr+1),QString::number(m_NextPortNr));

    QProcess* newSopcastProcess = new QProcess(this);

    newSopcastProcess->start(program);
    connect(newSopcastProcess, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
    connect(newSopcastProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(refreshStatus()));

    //m_SopCastProcesses.append( newSopcastProcess );
    PlayEntry entry;
    entry.m_Soplink = ui->text_soplink->text();
    entry.m_Name = ui->textName->text();
    entry.m_SopCastProces = newSopcastProcess;
    entry.m_OutboundPort = m_NextPortNr;

    m_PlayEntries.append(entry);

    m_NextPortNr += 2;//increment by 2

    refreshStatus();
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

    for(int i=0; i<m_PlayEntries.size(); i++)
    {
        if (m_PlayEntries.at(i).m_SopCastProces->state() == QProcess::NotRunning)
        {
            //restart it?...
            m_PlayEntries.at(i).m_SopCastProces->waitForFinished();

            QString program = QString("sp-sc %1 %2 %3").arg( m_PlayEntries.at(i).m_Soplink,
                                  QString::number(m_PlayEntries.at(i).m_OutboundPort+1),QString::number(m_PlayEntries.at(i).m_OutboundPort));

            m_PlayEntries.at(i).m_SopCastProces->start(program);
        }
    }

    refreshStatus();
}


void MainWindow::refreshStatus()
{

    //run this in a different thread
    refreshBufferStatusses();

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
        QTableWidgetItem *itemPort = new QTableWidgetItem();
        QTableWidgetItem *itemPlay = new QTableWidgetItem();
        QTableWidgetItem *itemDelete = new QTableWidgetItem();

        itemName->setText(m_PlayEntries.at(i).m_Name);
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
        pgbar->setValue( m_PlayEntries.at(i).m_BuffLevel );
        pgbar->setMaximum(100);

        itemPort->setText( QString::number(m_PlayEntries.at(i).m_OutboundPort));

        //changing color, works
#if 0
        QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );border-bottom-right-radius: 5px;border-bottom-left-radius: 5px;border: .px solid black;}";
        pgbar->setStyleSheet(danger);
#endif

        itemDelete->setIcon(QIcon(":/icons/delete.png"));
        ui->tableInfo->setItem(i, colName , itemName);
        ui->tableInfo->setItem(i, colSopLink , itemSopLink);
        ui->tableInfo->setItem(i, colStatus , itemStatus);
        ui->tableInfo->setCellWidget(i, colBuffer , pgbar);
        ui->tableInfo->setItem(i, colPortNr , itemPort);
        ui->tableInfo->setItem(i, colPlay , itemPlay);
        ui->tableInfo->setItem(i, colDelete , itemDelete);

    }
#endif
}

void MainWindow::refreshBufferStatusses()
{
    for(int i=0; i<m_PlayEntries.size(); i++)
    {
        if (m_PlayEntries.at(i).m_SopCastProces->state() == QProcess::Running)
        {
            updateBuffStatus(i);
        }
    }
}

void MainWindow::updateBuffStatus(int connectionId)
{
    QTcpSocket sock;
    sock.connectToHost("127.0.0.1", m_PlayEntries.at(connectionId).m_OutboundPort);
    if (sock.waitForConnected(500))
    {
        //connected, send status\n
        sock.write("state\ns\n");
        sock.waitForBytesWritten();
        sock.write("state\ns\n");
        sock.waitForBytesWritten();

        //QThread::msleep(50);

        sock.waitForReadyRead(1500);
        QByteArray replyBytes = sock.readLine();
        QString reply(replyBytes);

        QStringList stats = reply.split(" ");
        if (stats.size()>0)
        {
            //QMessageBox::warning(0,"Reply", stats[0]);
            m_PlayEntries[connectionId].m_BuffLevel = stats[0].toInt();
        }

        sock.disconnectFromHost();
    }
}
