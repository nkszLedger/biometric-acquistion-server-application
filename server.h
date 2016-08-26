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
    /*!
     * \brief Server
     * \param parent
     */
    explicit Server(QObject *parent = 0);
    /*!
     * \brief loadconfigFile
     */
    void loadconfigFile();
    /*!
     * \brief StartServer
     */
    void StartServer();

protected:
    /*!
     * \brief incomingConnection
     * \param socketDescriptor
     */
    void incomingConnection(qintptr socketDescriptor);

signals:

public slots:

private:
    QString output_path_;

};

#endif // SERVER_H
