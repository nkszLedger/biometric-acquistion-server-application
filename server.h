#ifndef SERVER_H
#define SERVER_H


#include <client.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void loadconfigFile();
    void StartServer();

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:

public slots:

private:
    QString output_path_;

};

#endif // SERVER_H
