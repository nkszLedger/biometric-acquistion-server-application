#include <QFile>
#include "server.h"
#include <shared.h>
#include <QString>

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    // load config file
    loadconfigFile();
}

void Server::loadconfigFile()
{
    int counter = 0;
    QString in_line;

    // load config file where executable resides
    QFile file("BioAcqServer2016.conf");
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            in_line = in.readLine();

            if(QString::compare(in_line.at(0), QString("#"), Qt::CaseInsensitive) == 0)
             continue;
            else
            {
                if(counter == 0)
                {
                    temp_path_global = in_line;
                }
                else
                {
                    output_path_global = in_line;
                }
                counter++;
            }
        }

        qDebug() << "temp_path_global: " << temp_path_global;
        qDebug() << "output_path_global: " << output_path_global;
        file.close();
    }
}

void Server::StartServer()
{
    //listen for incomming connections
    if(listen(QHostAddress::Any,1234))
    {
        qDebug() << "Server Started";
    }
    else
    {
        qDebug() << "Server NOT Started!";
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    // create a new client
    Client *client = new Client(this);
    // tag client to new id
    client->SetSocket(socketDescriptor);
}
