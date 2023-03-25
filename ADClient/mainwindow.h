#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWebSocket>
#include<QJsonObject>
#include<QJsonDocument>
#include <QMainWindow>
#include    <QTcpSocket>
#include    <QLabel>
#include    <QHostAddress>
#include    <QHostInfo>
#include<QFile>
#include<QFileInfo>
#include <QFileDialog>
#include<QDateTime>
#include<QtConcurrent/QtConcurrent>
#include<QMessageBox>
//#include "WebSender.h"
//static QWebSocket tcpClient;
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
private slots:
    void    onConnected();
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    //QWebSocket  *tcpClient;  //socket
    QSharedPointer<QWebSocket> tcpClient;
    //QWebSocket websocket;
    QString  msg;
    QByteArray  fileBase64;
    QString fileName;
    QString filePath;
    const int filepd=100000;
    //const QString addr="127.0.0.1";
    const QString addr="121.5.159.6";
    //const QString addr="localhost";
    const QString port="5055";
    QString name;
    long long filesize;
    int filenum;
    QString filesuffix;
    bool Sendfile();
    void ontextMessageReceived(QString);
    QWaitCondition  newdataAvailable;
    QMutex  mutex;
    QEventLoop loop;
    QTimer timer;
    void onsendBinaryMessage(QByteArray data);
    void onsendTextMessage(QString data);

signals:
    void sendBinaryMessage(QByteArray data);
    void sendTextMessage(QString data);
};
#endif // MAINWINDOW_H
