#include "client.h"
#include <shared.h>
#include <task.h>
#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QIODevice>

Client::Client(QObject *parent) :
    QObject(parent)
{
    // declaring thread pool for clients
    QThreadPool::globalInstance()->setMaxThreadCount(30);

    // clear variables
    file_names_.clear();
    file_sizes_.clear();

    // initialise
    no_files_ = 0;
    size_counter_ = 0;
    file_size_counter_ = 0;
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

void Client::notifyClientOnClose()
{
    // notify client on server going down
    socket->write( QString("#!0").toStdString().c_str() );

    qDebug() << "Client::notifyClientOnClose() - Notification Status of flush: " \
             << socket->flush();
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
        // Append data to lists
        file_names_.append(in_list.at(index));
        file_sizes_.append(in_list.at(index+1));
        index += 2;
    }


    for(int i = 0; i < no_files_; i++)
    {
        qDebug() << i;
        qDebug() << file_names_.at(i)<< " - " << file_sizes_.at(i);
    }
    // set file name to first
    setFileName(0);

    // write to client of successful header recieved
    socket->write(QString("#!1%1").arg(file_names_.size()).toStdString().c_str());
}

void Client::setFileName(int index)
{
    // sets the file name with appropriate file path at index specified
    current_file_name_ = temp_path_global;
    current_file_name_.append(QString("%1#").arg(socket->socketDescriptor()));
    current_file_name_.append(file_names_.at(index));
}

void Client::getEncryptedFiles(QByteArray data)
{
    // increment file size being recieved
    file_size_counter_ += data.size();

    // write file in append form
    QFile* entry = new QFile(current_file_name_);
    if(entry->open(QIODevice::Append))
    {
        // write to file
        entry->write(data);
        entry->close();
    }
    // delete memory
    delete entry;

    // check if file is complete
    if(QString::compare( QString("%1").arg(file_size_counter_), \
                         file_sizes_.at(current_file_index_), \
                         Qt::CaseInsensitive) == 0)
    {
        // - reset file size counter
        file_size_counter_ = 0;

        // - increment index of files
        current_file_index_++;

        // - check if next file exists
        if(current_file_index_ < file_sizes_.size())
        {
            // -- set file name to next
            setFileName(current_file_index_);

            // -- write to client of successful file recieved
            socket->write(QString("#!2").toStdString().c_str());
        }
        else
        {
            // in new thread

            //Time Consumer Functionality
            Task *mytask = new Task();
            mytask->setSocketInput(socket->socketDescriptor());
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
        // - get header from client
        getHeader(in_line);
        return;
    }
    else if(QString::compare(in_line.mid(0, 3), "#!4", Qt::CaseInsensitive) == 0)
    {
        // - get header from client
        //getHeader(in_line);
        //return;

        qDebug() <<" Client::readyRead() - Biometrics requested ";
        qDebug() <<" Client::readyRead() - With header " << in_line;
        retrieveRequestedBioModalities(in_line);
    }
    else if(QString::compare(in_line.mid(0, 3), "#!6", Qt::CaseInsensitive) == 0)
    {
        if( !requested_data_->isEmpty() )
            uploadRequestedBioModalities();

        requested_data_->clear();
    }
    else
    {
        // - read data
        getEncryptedFiles(data);
    }
}

void Client::retrieveRequestedBioModalities(QString in_line)
{
    int pass_header_index = 4;
    // extract modality data
    QString meta_data = in_line.mid(pass_header_index);
    qDebug() << "Extracted meta_data \n" << meta_data;

    QStringList requested_modalities_list = meta_data.split("#");
    qDebug() << "Gather these modalities \n" << requested_modalities_list;

    //Time Consumer Functionality
    Task *mytask = new Task();
    mytask->setSocketInput(socket->socketDescriptor());
    mytask->setAutoDelete(true);
    mytask->setAutoRetrieve(requested_modalities_list,true);
    connect( mytask, \
             SIGNAL(requestedModalitiesReady(QString)), \
             SLOT(sendEncryptedFile(QString)), Qt::QueuedConnection);

    QThreadPool::globalInstance()->start(mytask);
}


void Client::uploadRequestedBioModalities()
{
    // -----------------------------------------------------------------------
    qDebug() << "Client::sendEncryptedFile() - Status of socket: " \
             << socket->write( *requested_data_ );
    qDebug() << "Client::sendEncryptedFile() - Status of flush: " \
             << socket->flush();

    if( !socket->waitForBytesWritten() )
    {
        qDebug() << "Client::sendEncryptedFile() - Unable to flush...";
        qDebug() << "Client::sendEncryptedFile() - Sending a fail to send signal: " \
                 << socket->write( QString("#!0").toLatin1() );
        qDebug() << "Client::sendEncryptedFile() - Status of flush for failed signal: " \
                 << socket->flush();
    }
    else
    {
        qDebug() << "Client::sendEncryptedFile() - Requested data sent";
    }
}

void Client::TaskResult()
{
    qDebug() << "HERE 1";
    socket->write(QString("#!3").toStdString().c_str());
}

void Client::sendEncryptedFile(QString requestedModalitiesFilePath)
{
    requested_biometrics_file_ = new QFile( requestedModalitiesFilePath );

    qDebug() << "Client::sendEncryptedFile() - file QIODevice:path : " \
             << requestedModalitiesFilePath \
             << " with size: " << requested_biometrics_file_->size();

    if( requested_biometrics_file_->exists() )
    {
        if ( requested_biometrics_file_->open(QIODevice::ReadOnly) )
        {
             requested_data_ = new QByteArray(QByteArray::fromRawData( requested_biometrics_file_->readAll(), \
                                                        requested_biometrics_file_->size() ));

             requested_biometrics_file_->close();
        }

        if( requested_data_->isEmpty() )
        {
            qDebug() << "Client::sendEncryptedFile() - The byte array is empty ";
        }
        else
        {
            QString data_size = QString::number( requested_data_->size() );
            QString notification_header_and_datas_size = "#!4" + data_size;

            qDebug() << "Client::sendEncryptedFile() - Notification header to be sent: " \
                     << notification_header_and_datas_size;

            // notify receiver of the file size
            socket->write( QString(notification_header_and_datas_size).toLatin1() );

            qDebug() << "Client::sendEncryptedFile() - Notification Status of flush: " \
                     << socket->flush();
        }
    }
    else
    {
        qDebug() << "Client::sendEncryptedFile() - No file found to send";
    }

    // free memory
    //delete requested_modalities_zip_file;

    requested_data_->clear();
    delete requested_data_;
}
