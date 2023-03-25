#include "WebSender.h"

WebSender::WebSender()
{
    tcpClient.reset(new QWebSocket());
    QUrl Url=QString("ws://%1:%2").arg(addr).arg(port);
    tcpClient->open(Url);
}

WebSender::~WebSender()
{

}

WebSender &WebSender::GetInstance()
{
    static WebSender websender;
    return websender;
}

void WebSender::run()
{
    //QWebSocket websocket;
    //tcpClient.reset(new QWebSocket());
    QUrl Url=QString("ws://%1:%2").arg(addr).arg(port);
    //tcpClient->open(Url);
    qDebug()<<Url;
    qDebug()<<"Thread "<<QThread::currentThreadId();
    //websocket.open(Url);
    while(tcpClient->state()!=QAbstractSocket::ConnectedState)
    {
        qDebug()<<"Thread "<<tcpClient->state();
        //qDebug()<<"Thread w"<<websocket.state();
        QThread::msleep(1000);
    }

    connect(tcpClient.get(),&QWebSocket::textMessageReceived,this,&WebSender::ontextMessageReceived);
    QJsonObject json;
    json.insert("Name", name);
    json.insert("FileName",fileName);
    json.insert("FileSuffix",filesuffix);
    json.insert("FileStatus",true);
    json.insert("FileNum",filenum);

    if(tcpClient->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact))==0)
    {
        //QMessageBox::warning(this, "温馨提示", "文件发送失败！！！");
        emit MessageBoxtext("文件发送失败！！！");
        return;
    }
    emit pushButtontext(false);
    mutex.lock();
    newdataAvailable.wait(&mutex);
    mutex.unlock();
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"文件打开成功";

        QByteArray byte;
        for(int i=0;i<filenum;i++)
        {
            byte=file.read(filepd);
            //qDebug()<<"321";
            tcpClient->sendBinaryMessage(byte);
            mutex.lock();
            newdataAvailable.wait(&mutex);
            mutex.unlock();
            //ui->plainTextEdit->appendPlainText(byte.data());
            //ui->progressBar->setValue((i*1.0/filenum*100));
        }
    }
    else
    {
        //QMessageBox::warning(this, "温馨提示", "文件无法打开！！！");
        emit MessageBoxtext("文件无法打开！！！");
        qDebug()<<"文件没打开";
    }
    file.close();
    QJsonObject json1;
    json1.insert("Name", name);
    json1["FileStatus"]=false;
    tcpClient->sendTextMessage(QJsonDocument(json1).toJson(QJsonDocument::Compact));

    tcpClient->close();
    tcpClient.reset();
}

QString WebSender::getPort() const
{
    return port;
}

void WebSender::setPort(const QString &value)
{
    port = value;
}

QString WebSender::getAddr() const
{
    return addr;
}

void WebSender::setAddr(const QString &value)
{
    addr = value;
}

QString WebSender::getFilesuffix() const
{
    return filesuffix;
}

void WebSender::setFilesuffix(const QString &value)
{
    filesuffix = value;
}

int WebSender::getFilenum() const
{
    return filenum;
}

void WebSender::setFilenum(int value)
{
    filenum = value;
}

long long WebSender::getFilesize() const
{
    return filesize;
}

void WebSender::setFilesize(long long value)
{
    filesize = value;
}

QString WebSender::getName() const
{
    return name;
}

void WebSender::setName(const QString &value)
{
    name = value;
}

QString WebSender::getFilePath() const
{
    return filePath;
}

void WebSender::setFilePath(const QString &value)
{
    filePath = value;
}

QString WebSender::getFileName() const
{
    return fileName;
}

void WebSender::setFileName(const QString &value)
{
    fileName = value;
}

void WebSender::ontextMessageReceived(QString data)
{
    QJsonDocument reqDoc = QJsonDocument::fromJson(data.toUtf8());
    if(reqDoc.isObject())
    {
        QJsonObject req = reqDoc.object();
        auto type=req.value("Type").toString();
        if(type=="OK")
        {
            emit MessageBoxtext("文件开始发送！！！");
            newdataAvailable.wakeAll();
        }
        else if(type=="Rate")
        {
            newdataAvailable.wakeAll();
        }
    }
    emit textMessageReceived(data);
}
