#include <QFile>
#include <QString>
#include "client.h"
#include <shared.h>
#include <QFileInfo>

Client::Client(QObject *parent) :
    QObject(parent)
{
    QThreadPool::globalInstance()->setMaxThreadCount(15);

    file_names_.clear();
    file_sizes_.clear();

    no_files_ = 0;
    size_counter = 0;
    file_size_counter = 0;
    current_file_index_ = 0;

}

void Client::SetSocket(qintptr socketDescriptor)
{
    // initialise socket
    socket = new QTcpSocket(this);

    // signals and slots declaration
    connect(socket,SIGNAL(connected()),   this,SLOT(connected()));
    connect(socket,SIGNAL(readyRead()),   this,SLOT(readyRead()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    // set socket
    socket->setSocketDescriptor(socketDescriptor);
}


void Client::connected()
{
    qDebug() << "client connected event";
}

void Client::disconnected()
{
    qDebug() << "client disconnected";
}


/*!
 * \brief Client::getHeader Header begins with '#!1'
 * seperated with '#' no_files, file_name, file_size, file_name, file_size, ...
 * \param inLine
 */
void Client::getHeader(QString inLine)
{
    QStringList in_list;
    in_list = inLine.split("#");

    no_files_ = QString(in_list.at(2)).toInt();

    // remove extra meta data
    for(int i = 0; i <=2; i++)
        in_list.removeAt(0);

    int index = 0;
    for(int i = 0; i < no_files_; i++)
    {
        file_names_.append(in_list.at(index));
        file_sizes_.append(in_list.at(index+1));
        index += 2;
    }

    // set file name to first
    setFileName(0);

    // write to client of successful header recieved
    socket->write(QString("#!1%1").arg(file_names_.size()).toStdString().c_str());
}

void Client::setFileName(int index)
{
    current_file_name_ = temp_path_global;
    current_file_name_.append(QString("%1#").arg(socket->socketDescriptor()));
    current_file_name_.append(file_names_.at(index));
}

void Client::getEncryptedFiles(QByteArray data)
{
    // increment file size being recieved
    file_size_counter += data.size();

    // write file
    QFile* entry = new QFile(current_file_name_);
    if(entry->open(QIODevice::Append))
    {
        entry->write(data);
        entry->close();
    }
    delete entry;

    // check if file is complete
    if(QString::compare(QString("%1").arg(file_size_counter), file_sizes_.at(current_file_index_), Qt::CaseInsensitive) == 0)
    {
        // reset file size counter
        file_size_counter = 0;

        qDebug() << "File completed: " << current_file_index_;

        // increment index of files
        current_file_index_++;

        // check if next file exists
        if(current_file_index_ < file_sizes_.size())
        {
            // - set file name to next
            setFileName(current_file_index_);

            // write to client of successful file recieved
            socket->write(QString("#!2").toStdString().c_str());
        }
        else
        {
            qDebug() << "Transmission ended.. to decrypt and merge...";

            // in new thread

            //Time Consumer
            Task *mytask = new Task();
            mytask->setSocketInput(socket->socketDescriptor());
            mytask->setPassPhrase("MDSBRMP");
            mytask->setAutoDelete(true);
            connect(mytask,SIGNAL(completed()),SLOT(TaskResult()), Qt::QueuedConnection);
            QThreadPool::globalInstance()->start(mytask);

        }

    }
}

void Client::readyRead()
{  
    // read all data
    QByteArray data = socket->readAll();
    // set to QString
    QString in_line = data;

    // check if Header (Meta Data)
    if(QString::compare(in_line.mid(0, 3), "#!1", Qt::CaseInsensitive) == 0)
    {
        qDebug() << "HEADER...";
        getHeader(in_line);
        return;
    }
    else
    {
        // read data
        getEncryptedFiles(data);
    }
}



void Client::TaskResult()
{
    qDebug() << "Completed Sending File (SLOT)";

    socket->write(QString("#!3").toStdString().c_str());
}
