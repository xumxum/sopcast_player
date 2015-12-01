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

    //ui->textName->setText("Channel Name");
    ui->text_soplink->setText("sop://");

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

    //1. Start the process
    QProcess* newSopcastProcess = new QProcess(this);

    newSopcastProcess->start(program);
    connect(newSopcastProcess, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
    connect(newSopcastProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(refreshStatus()));

    //2. Save it in our list
    PlayEntry entry;
    entry.m_Soplink = ui->text_soplink->text();
    entry.m_Name = ui->textName->text();
    entry.m_SopCastProces = newSopcastProcess;
    entry.m_OutboundPort = m_NextPortNr;

    m_DataMutex.lock();
    m_PlayEntries.append(entry);
    m_DataMutex.unlock();

    //3. Add new entry in the table
    int row = ui->tableInfo->rowCount();
    ui->tableInfo->insertRow(row);


    QTableWidgetItem *itemName= new QTableWidgetItem();
    QTableWidgetItem *itemSopLink = new QTableWidgetItem();
    QTableWidgetItem *itemStatus = new QTableWidgetItem();
    QTableWidgetItem *itemPort = new QTableWidgetItem();

    itemName->setText(entry.m_Name);
    itemSopLink->setText(entry.m_Soplink);

    ui->textName->setText("");
    ui->text_soplink->setText("sop://");
    itemStatus->setText( QString("Starting") );

    QProgressBar *pgbar = new QProgressBar();
    pgbar->setValue(0);
    pgbar->setMaximum(100);

    itemPort->setText( QString::number(m_NextPortNr) );

    QPushButton* buttonPlay = new QPushButton("Play", ui->tableInfo);
    buttonPlay->setIcon( QIcon(":/icons/play.png") );

    connect(buttonPlay, SIGNAL(clicked()), this, SLOT(cellPlayClicked()));

    QPushButton* buttonDelete = new QPushButton("Delete", ui->tableInfo);
    buttonDelete->setIcon( QIcon(":/icons/delete.png") );

    connect(buttonDelete, SIGNAL(clicked()), this, SLOT(cellDeleteClicked()));

    ui->tableInfo->setItem(row, colName , itemName);
    ui->tableInfo->setItem(row, colSopLink , itemSopLink);
    ui->tableInfo->setItem(row, colStatus , itemStatus);
    ui->tableInfo->setCellWidget(row, colBuffer , pgbar);
    ui->tableInfo->setItem(row, colPortNr , itemPort);
    ui->tableInfo->setCellWidget(row, colPlay , buttonPlay);
    ui->tableInfo->setCellWidget(row, colDelete , buttonDelete);


    m_NextPortNr += 2;//increment by 2

    refreshStatus();
}

void MainWindow::on_sopcast_exited(int exitStatus)
{
    m_DataMutex.lock();
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
    m_DataMutex.unlock();

    refreshStatus();
}


void MainWindow::refreshStatus()
{
    QMutexLocker ml(&m_DataMutex);

    //run this in a different thread?
    refreshBufferStatusses();

    //ui->tableInfo->setShowGrid(false);
#if 1
    for(int i=0; i<m_PlayEntries.size(); i++)
    {
        QProcess* processSopCast = m_PlayEntries.at(i).m_SopCastProces;
        if (processSopCast)
        {
            QTableWidgetItem* itemStatus = ui->tableInfo->item(i, colStatus);
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

        QProgressBar *pgbar = dynamic_cast<QProgressBar*>(ui->tableInfo->cellWidget(i, colBuffer));
        if (pgbar) pgbar->setValue( m_PlayEntries.at(i).m_BuffLevel );

        //if (pgbar) pgbar->setValue( (pgbar->value() + 1) %101);

        //changing color, works
#if 0
        QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );border-bottom-right-radius: 5px;border-bottom-left-radius: 5px;border: .px solid black;}";
        pgbar->setStyleSheet(danger);
#endif

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

void MainWindow::cellDeleteClicked()
{
    QMutexLocker ml(&m_DataMutex);

    for(int index=0; index<m_PlayEntries.size(); index++)
    {
        if (ui->tableInfo->cellWidget(index, colDelete) == sender())
        {
            //found our button
            ui->tableInfo->removeRow(index);

            //send term, wait a bit then kill if it's dead
            //first disable the signals , no need for them anymore for this process
            disconnect(m_PlayEntries[index].m_SopCastProces, SIGNAL(finished(int)), this, SLOT(on_sopcast_exited(int)));
            disconnect(m_PlayEntries[index].m_SopCastProces, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(refreshStatus()));

            m_PlayEntries[index].m_SopCastProces->terminate();
            if (!m_PlayEntries[index].m_SopCastProces->waitForFinished(500))
                m_PlayEntries[index].m_SopCastProces->kill();

            //do I need this or qt will take care? ..
            delete m_PlayEntries[index].m_SopCastProces;

            m_PlayEntries.remove(index);

            break;
        }
    }
}


void MainWindow::cellPlayClicked()
{
    QMutexLocker ml(&m_DataMutex);

    for(int index=0; index<m_PlayEntries.size(); index++)
    {
        if (ui->tableInfo->cellWidget(index, colPlay) == sender())
        {
            //found our button
            if (m_PlayerProces.state() != QProcess::NotRunning)
            {
                //still running, kill it
                m_PlayerProces.terminate();
                if (!m_PlayerProces.waitForFinished(500))
                    m_PlayerProces.kill();
            }

            QString program = QString("vlc http://127.0.0.1:%1/tv.asf").arg(m_PlayEntries[index].m_OutboundPort);

            m_PlayerProces.start(program);

            break;
        }
    }
}
