#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QJsonObject>
#include<QJsonDocument>
#include <QtNetwork>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMainWindow>
#include <QTcpServer>
#include <QLabel>
#include <QDateTime>
#include <QMutex>
#include<QMutexLocker>
#include<QtConcurrent/QtConcurrent>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void    closeEvent(QCloseEvent *event);
private:
    Ui::MainWindow *ui;
    QLabel  *LabListen;//状态栏标签
    QLabel  *LabSocketState;//状态栏标签
    QMutex mutex;
    //QWebSocketServer *tcpServer; //TCP服务器

    QSharedPointer<QWebSocketServer> tcpServer;

    QVector<QWebSocket*> tcpSocket;//TCP通讯的Socket

    //QVector<QSharedPointer<QWebSocket>> tcpSocket;

    QString     IP="0.0.0.0";
    quint16     port=5055;//端口

    QString getLocalIP();//获取本机IP地址
    //QMap<QWebSocket*,QByteArray> fileBase64map;
    QString  curPath;
    QDir dir;
    void onsend(QWebSocket *socket,const QString &str);
    void onlog(const QString &str);
private slots:
    //自定义槽函数
    void onNewConnection();
    void onTextMessageReceived(QString);
    void onDisconnected();
    void onbinaryMessageReceived(QByteArray);
signals:
    void send(QWebSocket *socket,const QString &str);
};
#endif // MAINWINDOW_H
