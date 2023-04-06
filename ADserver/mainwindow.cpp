#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    LabListen=new QLabel("监听状态:");
    LabListen->setMinimumWidth(150);
    ui->statusbar->addWidget(LabListen);
    ui->plainTextEdit->setReadOnly(true);
    LabSocketState=new QLabel("Socket状态：");//
    LabSocketState->setMinimumWidth(200);
    ui->statusbar->addWidget(LabSocketState);
    curPath=QDir::currentPath();
    dir.mkdir("信管B201");

    curPath=curPath+R"(/信管B201/)";
    dir.setPath(curPath);
    //qDebug()<<curPath;
    //qDebug()<<dir.path();
    QString localIP=getLocalIP();//本机IP
    this->setWindowTitle("本机IP："+localIP);
    //ui->comboIP->addItem(localIP);

    tcpServer.reset(new QWebSocketServer("server", QWebSocketServer::NonSecureMode));

    connect(tcpServer.get(),SIGNAL(newConnection()),this,SLOT(onNewConnection()));
    connect(this,&MainWindow::send,this,&MainWindow::onsend);
    IP=localIP;
    QHostAddress addr(IP);
    tcpServer->listen(QHostAddress::Any,port);//
    //    tcpServer->listen(QHostAddress::LocalHost,port);// Equivalent to QHostAddress("127.0.0.1").
//    ui->plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
//    ui->plainTextEdit->appendPlainText("**开始监听...");
//    ui->plainTextEdit->appendPlainText("**服务器地址："
//                                       +tcpServer->serverAddress().toString());
//    ui->plainTextEdit->appendPlainText("**服务器端口："
//                                       +QString::number(tcpServer->serverPort())+"\n");
    onlog(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
    onlog("**开始监听...");
    onlog("**服务器地址："+tcpServer->serverAddress().toString());
    onlog("**服务器端口："+QString::number(tcpServer->serverPort())+"\n");
    LabListen->setText("监听状态：正在监听");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //关闭窗口时停止监听
    if (tcpServer->isListening())
        tcpServer->close();//停止网络监听
    event->accept();
}

QString MainWindow::getLocalIP()
{
    //获取本机IPv4地址
    QString hostName=QHostInfo::localHostName();//本地主机名
    QHostInfo   hostInfo=QHostInfo::fromName(hostName);
    QString   localIP="";

    QList<QHostAddress> addList=hostInfo.addresses();//

    if (!addList.isEmpty())
        for (int i=0;i<addList.count();i++)
        {
            QHostAddress aHost=addList.at(i);
            if (QAbstractSocket::IPv4Protocol==aHost.protocol())
            {
                localIP=aHost.toString();
                break;
            }
        }
    return localIP;
}

void MainWindow::onsend(QWebSocket *socket,const QString &str)
{
    socket->sendTextMessage(str);
}

void MainWindow::onlog(const QString &str)
{
    QMutexLocker locker(&mutex);
    ui->plainTextEdit->appendPlainText(str);
    QFile file("服务器日志.txt");
    if(file.open(QIODevice::Append|QIODevice::WriteOnly|QIODevice::Text))
    {
        file.write(str.toUtf8()+"\n");
        file.close();
    }
}

void MainWindow::onNewConnection()
{
    QWebSocket *newSocket = new QWebSocket();
    newSocket=tcpServer->nextPendingConnection();
//    QSharedPointer<QWebSocket> newSocket;
//    newSocket.reset(tcpServer->nextPendingConnection());
//    ui->plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
//    ui->plainTextEdit->appendPlainText("**有新的用户链接：");
//    ui->plainTextEdit->appendPlainText("**他/她的IP是："+
//                                       newSocket->peerAddress().toString());
//    ui->plainTextEdit->appendPlainText("**他/她的端口是："+
//                                       QString::number(newSocket->peerPort())+"\n");
    onlog(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
    onlog("**有新的用户链接：");
    onlog("**他/她的IP是："+newSocket->peerAddress().toString());
    onlog("**他/她的端口是："+QString::number(newSocket->peerPort())+"\n");
//    connect(newSocket.get(), SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
//    connect(newSocket.get(), SIGNAL(disconnected()), this, SLOT(onDisconnected()));
//    connect(newSocket.get(),&QWebSocket::binaryMessageReceived, this,&MainWindow::onbinaryMessageReceived);

    connect(newSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(newSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(newSocket,&QWebSocket::binaryMessageReceived, this,&MainWindow::onbinaryMessageReceived);
    newSocket->setProperty("FileName","");
    newSocket->setProperty("FilePath","");
    newSocket->setProperty("FileSuffix","");
    newSocket->setProperty("FileStatus",false);
    newSocket->setProperty("FileNum",0);
    newSocket->setProperty("Name","");
    tcpSocket.push_back(newSocket);
}

void MainWindow::onTextMessageReceived(QString data)
{
    QJsonDocument reqDoc = QJsonDocument::fromJson(data.toUtf8());
    if(reqDoc.isObject())
    {
        QJsonObject req = reqDoc.object();
        QString name=req.value("Name").toString();
        if(!name.isEmpty())
        {
            dir.mkdir(name);
            QWebSocket *oldSocket = (QWebSocket *) sender();
//            ui->plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
//            ui->plainTextEdit->appendPlainText("当前发送文件的IP:"+
//                                               oldSocket->peerAddress().toString());
//            ui->plainTextEdit->appendPlainText("当前发送文件的端口:"+
//                                               QString::number(oldSocket->peerPort()));
//            ui->plainTextEdit->appendPlainText("他/她的名字是:"+
//                                               name);

            onlog(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
            onlog("当前发送文件的IP:"+oldSocket->peerAddress().toString());
            onlog("当前发送文件的端口:"+QString::number(oldSocket->peerPort()));
            onlog("他/她的名字是:"+name);
            if(req.contains("FileStatus"))
            {
                auto FileStatus=req.value("FileStatus").toBool();
                if(FileStatus)
                {

                    auto FileName=req.value("FileName").toString();
                    auto FileSuffix=req.value("FileSuffix").toString();
                    auto FileNum=req.value("FileNum").toInt();
                    oldSocket->setProperty("Name",name);
                    oldSocket->setProperty("FileName",FileName);
                    oldSocket->setProperty("FileSuffix",FileSuffix);
                    oldSocket->setProperty("FileNum",FileNum);


                    auto FilePath=curPath+name+R"(/)"+FileName+'.'+FileSuffix;
                    auto FilePath1=curPath+name+R"(/)"+FileName+".pf";
                    int i=0;
                    while(QFile::exists(FilePath)||QFile::exists(FilePath1))
                    {
                        i++;
                        //FileName=FileName+QString::number(i);
                        FilePath=curPath+name+R"(/)"+FileName+QString::number(i)+'.'+FileSuffix;
                        FilePath1=curPath+name+R"(/)"+FileName+QString::number(i)+".pf";
                    }
                    if(i==0)
                    {
                        FilePath=curPath+name+R"(/)"+FileName;
                    }
                    else
                    {
                        FilePath=curPath+name+R"(/)"+FileName+QString::number(i);
                    }

                    oldSocket->setProperty("FilePath",FilePath);
                    oldSocket->setProperty("FileStatus",true);
                    ui->plainTextEdit->appendPlainText("正在发送的文件名:"+
                                                       FileName+'.'+FileSuffix+"\n");
                    QJsonObject json;
                    json.insert("Type","OK");
                    //oldSocket->sendTextMessage(QJsonDocument(json).toJson());
                    emit send(oldSocket,QJsonDocument(json).toJson());
                }
                else
                {
                    auto FilePath=oldSocket->property("FilePath").toString();
                    auto FileSuffix=oldSocket->property("FileSuffix").toString();
                    QFile::rename(FilePath+".pf",FilePath+'.'+FileSuffix);
                    ui->plainTextEdit->appendPlainText("文件存储路径为:"+
                                                       FilePath+'.'+FileSuffix+"\n");
                    oldSocket->setProperty("FileName","");
                    oldSocket->setProperty("FileSuffix","");
                    oldSocket->setProperty("FilePath","");
                    oldSocket->setProperty("FileStatus",false);
                    QJsonObject json;
                    json.insert("Type","FileOK");
                    //oldSocket->sendTextMessage(QJsonDocument(json).toJson());
                    emit send(oldSocket,QJsonDocument(json).toJson());
                }
            }




            //            QString fileName = req.value("FileName").toString();
            //            QByteArray fileValue = QByteArray::fromBase64(req.value("FileBase64").toVariant().toByteArray());
            //            QString  curPath = QDir::currentPath();
            //            QDir dir(curPath);
            //            QString sub = "信管B201";//"保存文件";
            //            dir.mkdir(sub);
            //            dir.setPath(curPath+"/"+sub);
            //            dir.mkdir(name);
            //            QFile file(curPath + "/" + sub + "/"+name+"/"+fileName);
            //            file.open(QFile::WriteOnly);
            //            QDataStream in(&file);
            //            in.writeRawData(fileValue, fileValue.size());
            //            file.close();
            //QString format = tr("%1\n[%2]的%3已保存至\n%4").arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss")).arg(name).arg(fileName).arg(curPath + "/" + sub + "/"+name+"/");
            //ui->plainTextEdit->appendPlainText(format);
        }
    }
}

void MainWindow::onDisconnected()
{
    //QMutexLocker locker(&mutex);
    QWebSocket *oldSocket = qobject_cast<QWebSocket *>(sender());
    //QSharedPointer<QWebSocket> oldSocket;
    //oldSocket.reset(qobject_cast<QWebSocket *>(sender()));
//    ui->plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
//    ui->plainTextEdit->appendPlainText("**有用户断开链接：");
//    ui->plainTextEdit->appendPlainText("**他/她的IP是："+
//                                       oldSocket->peerAddress().toString());
//    ui->plainTextEdit->appendPlainText("**他/她的端口是："+
//                                       QString::number(oldSocket->peerPort()));

    onlog(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
    onlog("**有用户断开链接：");
    onlog("**他/她的IP是："+oldSocket->peerAddress().toString());
    onlog("**他/她的端口是："+QString::number(oldSocket->peerPort()));
    if(!oldSocket->property("Name").toString().isEmpty())
    {
//        ui->plainTextEdit->appendPlainText("他/她的名字是:"+
//                                           oldSocket->property("Name").toString()+"\n");
        onlog("他/她的名字是:"+oldSocket->property("Name").toString()+"\n");
    }
    else
    {
        //ui->plainTextEdit->appendPlainText("\n");
        onlog("\n");
    }
    //qDebug()<<tcpSocket.size();
    //        for(auto it = this->tcpSocket.begin(); it != tcpSocket.end(); ++it)
    //        {
    //            if(oldSocket == *it) {
    //                //tcpSocket.erase(it);
    //                qDebug()<<tcpSocket.size();
    //                break;
    //            }
    //        }
    tcpSocket.removeOne(oldSocket);
    //tcpSocket.erase(qFind(tcpSocket.begin(),tcpSocket.end(),oldSocket));
    oldSocket->deleteLater();
    //oldSocket.reset();
    //delete oldSocket;
    //qDebug()<<tcpSocket.size();
}

void MainWindow::onbinaryMessageReceived(QByteArray byte)
{

    QWebSocket *oldSocket = qobject_cast<QWebSocket *>(sender());
    if(oldSocket->property("FileStatus").toBool())
    {
        QString FilePath=oldSocket->property("FilePath").toString();
        if(!FilePath.isEmpty())
        {
            QFile file(FilePath+".pf");
            if(file.open(QIODevice::Append|QIODevice::WriteOnly))
            {
                //ui->plainTextEdit->appendPlainText(byte.data());
                file.write(byte);
                file.close();
                int FileNum=oldSocket->property("FileNum").toInt();
                FileNum-=1;
                oldSocket->setProperty("FileNum",FileNum);
                QJsonObject json;
                json.insert("Type","Rate");
                json.insert("Progress",FileNum);
                //oldSocket->sendTextMessage(QJsonDocument(json).toJson());
                emit send(oldSocket,QJsonDocument(json).toJson());
            }
            else
            {
                //ui->plainTextEdit->appendPlainText(FilePath+".pf");
                //ui->plainTextEdit->appendPlainText(tr("文件没打开：%1").arg(FilePath+".pf"));
                onlog(tr("文件没打开：%1").arg(FilePath+".pf"));
            }
        }
    }

}
