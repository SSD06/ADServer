#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpClient.reset(new QWebSocket()); //创建socket变量
    ui->pushButton->setEnabled(false);
    ui->plainTextEdit->setReadOnly(true);
    QUrl Url=QString("ws://%1:%2").arg(addr).arg(port);
    tcpClient->open(Url);

    //websocket.open(Url);

    //tcpClient.open(Url);
    //WebSender &websender=WebSender::GetInstance();
//    WebSender::GetInstance().setAddr(addr);
//    WebSender::GetInstance().setPort(port);
    connect(tcpClient.get(),SIGNAL(connected()),this,SLOT(onConnected()));
    connect(tcpClient.get(),&QWebSocket::textMessageReceived,this,&MainWindow::ontextMessageReceived);
    connect(this,&MainWindow::sendBinaryMessage,this,&MainWindow::onsendBinaryMessage);
    connect(this,&MainWindow::sendTextMessage,this,&MainWindow::onsendTextMessage);
    //connect(&tcpClient,SIGNAL(connected()),this,SLOT(onConnected()));
    //connect(&tcpClient,&QWebSocket::textMessageReceived,this,&MainWindow::ontextMessageReceived);


//    connect(&websender,&WebSender::pushButtontext,this,[&](bool data){
//        ui->pushButton->setEnabled(data);
//    });
//    connect(&(WebSender::GetInstance()),&WebSender::pushButtontext,this,[&](bool data){
//        ui->pushButton->setEnabled(data);
//    });
//    connect(&(WebSender::GetInstance()),&WebSender::textMessageReceived,this,&MainWindow::ontextMessageReceived);
//    connect(&(WebSender::GetInstance()),&WebSender::MessageBoxtext,this,[&](QString data){
//        QMessageBox::warning(this, "温馨提示", data);
//    });


    connect(&timer,&QTimer::timeout,[&](){
        //qDebug()<<tcpClient->state();
        //qDebug()<<websocket.state();
        //websocket.open(Url);
        if(tcpClient->state()==QAbstractSocket::UnconnectedState)
        {
            QUrl Url=QString("ws://%1:%2").arg(addr).arg(port);
            tcpClient->open(Url);
        }

    });
    timer.start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (tcpClient->state()==QAbstractSocket::ConnectedState)
        tcpClient->close();

//    if (tcpClient.state()==QAbstractSocket::ConnectedState)
//        tcpClient.disconnect();
    //event->accept();
}

void MainWindow::onConnected()
{
    ui->plainTextEdit->appendPlainText("**已连接到服务器");
    //ui->plainTextEdit->appendPlainText("**peer address:"+tcpClient->peerAddress().toString());
    ui->plainTextEdit->appendPlainText("**端口为："+QString::number(tcpClient->peerPort())+"\n");
    //ui->plainTextEdit->appendPlainText("**端口为："+QString::number(tcpClient.peerPort()));
}


void MainWindow::on_pushButton_clicked()
{
    ui->progressBar->setValue(0);
    if(ui->lineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "温馨提示", "请输入学号！！！");
        return;
    }
    else if(ui->lineEdit->text().length()!=12)
    {
        QMessageBox::warning(this, "温馨提示", "请输入正确的学号！！！");
        return;
    }
    else if(ui->lineEdit_2->text().isEmpty())
    {
        QMessageBox::warning(this, "温馨提示", "请输入姓名！！！");
        return;
    }
    else if(ui->lineEdit_3->text().isEmpty())
    {
        QMessageBox::warning(this, "温馨提示", "请输入要重命名后的文件名！！！");
        return;
    }
    name=ui->lineEdit->text()+ui->lineEdit_2->text();
    auto fileName=ui->lineEdit_3->text();

//    WebSender::GetInstance().setName(name);
//    WebSender::GetInstance().setFileName(fileName);
//    qDebug()<<"main "<<QThread::currentThreadId();
//    WebSender::GetInstance().start();

//    auto fileName=ui->lineEdit_3->text();
    QJsonObject json;
    json.insert("Name", name);
    json.insert("FileName",fileName);
    json.insert("FileSuffix",filesuffix);
    json.insert("FileStatus",true);
    json.insert("FileNum",filenum);

    //json.insert("FileBase64",QJsonValue::fromVariant(fileBase64));




    if(tcpClient->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact))==0)
    {
        QMessageBox::warning(this, "温馨提示", "文件发送失败！！！");
        return;
    }
//        if(tcpClient.sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact))==0)
//        {
//            QMessageBox::warning(this, "温馨提示", "文件发送失败！！！");
//            return;
//        }
    ui->pushButton->setEnabled(false);


    //    json["Name"]=name;
    //    json["FileName"]=fileName;
    //    json["FileSuffix"]=filesuffix;

}

void MainWindow::on_pushButton_2_clicked()
{
    QString arg = "";
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "", "All Files(*);", &arg);
    if(filePath != "")
    {
        QFileInfo fileInfo(filePath);
        filesuffix=fileInfo.suffix();
        fileName=fileInfo.fileName();
        qDebug()<<fileInfo.baseName();
        filesize=fileInfo.size();
        filenum=filesize/filepd;
        if(!(filesize%filepd==0))
        {
            filenum+=1;
        }
        qDebug()<<"文件大小："<<filesize;
        qDebug()<<"文件传输次数："<<filenum;
//        WebSender::GetInstance().setFilenum(filenum);
//        WebSender::GetInstance().setFilesize(filesize);
//        WebSender::GetInstance().setFilePath(filePath);
//        WebSender::GetInstance().setFilesuffix(filesuffix);
        this->filePath=filePath;
        qDebug()<<filePath;
        ui->plainTextEdit->appendPlainText("你当前选取的文件为："+fileName+"\n");
        ui->pushButton->setEnabled(true);
    }

}

bool MainWindow::Sendfile()
{
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"文件打开成功";

        QByteArray byte;
        for(int i=0;i<filenum;i++)
        {
            mutex.lock();
            byte=file.read(filepd);
            //qDebug()<<"321";
            //tcpClient->sendBinaryMessage(byte);
            emit sendBinaryMessage(byte);
            //tcpClient.sendBinaryMessage(byte);

            newdataAvailable.wait(&mutex,200);
            mutex.unlock();
            //ui->plainTextEdit->appendPlainText(byte.data());
            //ui->progressBar->setValue((i*1.0/filenum*100));
        }
    }
    else
    {
        QMessageBox::warning(this, "温馨提示", "文件无法打开！！！");
        qDebug()<<"文件没打开";
        return false;
    }
    file.close();
    QJsonObject json;
    json.insert("Name", name);
    json["FileStatus"]=false;
    emit sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
    //tcpClient->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
    //tcpClient.sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
    return true;
}



void MainWindow::ontextMessageReceived(QString data)
{
    QJsonDocument reqDoc = QJsonDocument::fromJson(data.toUtf8());
    if(reqDoc.isObject())
    {
        QJsonObject req = reqDoc.object();
        auto type=req.value("Type").toString();
        if(type=="OK")
        {
//            if(!Sendfile())
//            {
//                return;
//            }
            QtConcurrent::run([&](){
                Sendfile();
            });
            //QMessageBox::information(this, "温馨提示", "文件开始发送！！！");
            ui->plainTextEdit->appendPlainText("正在发送文件："+fileName+"\n");

        }
        else if(type=="FileOK")
        {
            QMessageBox::information(this, "温馨提示", "文件发送成功！！！");
            QString format = tr("%1\n%2文件已经发送").arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss")).arg(name+"的"+fileName);
            ui->plainTextEdit->appendPlainText(format+"\n");
            ui->pushButton->setEnabled(true);
        }
        else if(type=="Rate")
        {
            //qDebug()<<"123";
            newdataAvailable.wakeAll();
            auto progress=req.value("Progress").toInt();
            //qDebug()<<"Progress"<<req.value("Progress").toInt();
            ui->progressBar->setValue(((filenum-progress)*1.0/filenum*100));


        }
    }

}

void MainWindow::onsendBinaryMessage(QByteArray data)
{
    tcpClient->sendBinaryMessage(data);
}

void MainWindow::onsendTextMessage(QString data)
{
    tcpClient->sendTextMessage(data);
}
