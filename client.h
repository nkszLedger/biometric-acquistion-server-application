#ifndef CLIENT_H
#define CLIENT_H

#include <task.h>
#include <QDebug>
#include <QObject>
#include <QTcpSocket>
#include <QThreadPool>

#include <QList>
#include <QStringList>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    void SetSocket(qintptr socketDescriptor);

    void getHeader(QString inLine);

    void getEncryptedFiles(QByteArray data);
    void setFileName(int index);
signals:

public slots:
    void connected();
    void disconnected();
    void readyRead();
    void TaskResult();

private:
    QTcpSocket *socket;

    int size_counter;

    QStringList file_names_;
    QStringList file_sizes_;
    int no_files_;
    int file_size_counter;
    int current_file_index_;
    QString current_file_name_;

};

#endif // CLIENT_H
