#ifndef WEBSENDER_H
#define WEBSENDER_H
#include<QThread>
#include<QWaitCondition>
#include<QMutex>
#include <QWebSocket>
#include<QJsonObject>
#include<QJsonDocument>
#include<QFile>
class WebSender:public QThread
{
    Q_OBJECT
public:
    WebSender();
    ~WebSender();
    static WebSender &GetInstance();
    QString getFileName() const;
    void setFileName(const QString &value);

    QString getFilePath() const;
    void setFilePath(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    long long getFilesize() const;
    void setFilesize(long long value);

    int getFilenum() const;
    void setFilenum(int value);

    QString getFilesuffix() const;
    void setFilesuffix(const QString &value);

    QString getAddr() const;
    void setAddr(const QString &value);

    QString getPort() const;
    void setPort(const QString &value);

protected:
    void run() Q_DECL_OVERRIDE;
private:
    QSharedPointer<QWebSocket> tcpClient;

    QString fileName;
    QString filePath;
    QString name;
    long long filesize;
    int filenum;
    QString filesuffix;
    QWaitCondition  newdataAvailable;
    QMutex  mutex;
    const int filepd=100000;
    //const QString addr="127.0.0.1";
    //const QString addr="121.5.159.6";
    QString addr="localhost";
    QString port="5055";
    void ontextMessageReceived(QString data);
signals:
    void textMessageReceived(QString data);
    void MessageBoxtext(QString data);
    void pushButtontext(bool data);

};

#endif // WEBSENDER_H
