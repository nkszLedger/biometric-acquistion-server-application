#include "storage.h"
#include "database.h"

#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

Storage::Storage(QObject *parent): QObject(parent)
{
    /* clear all variables */
    base_url_.clear();
    current_file_.clear();
    list_of_files_.clear();

    /* assign variables with values */
    file_index_ = 0;
    manager_    = new QNetworkAccessManager(this);

    connect( manager_, SIGNAL(finished(QNetworkReply*)), \
                this, SLOT(onFinished(QNetworkReply*)));

    /* populate updated list of files that exists*/
    readFileNamesFromDB();
}

void Storage::store(QString filePath)
{
    /* File Selective: regex to send specific data file */
    QRegularExpression regex_main_data_file("\\d+\\-\\d+\\_\\d+\\-\\d+\\-\\d+.zip");

    QDirIterator dir_it( QDirIterator( filePath,\
                                        ( QDir::Files | \
                                          QDir::NoDot | \
                                          QDir::NoDotDot | \
                                          QDir::NoDotAndDotDot \
                                        )));
    while( dir_it.hasNext() )
    {
        QString file_name_discovered =  dir_it.next();
        QFile file( filePath + "/" + file_name_discovered );
        QByteArray file_bytes;
        QRegularExpressionMatch match = regex_main_data_file.match( filePath + "/" + file_name_discovered );

        if( match.hasMatch() )
        {
            if( file.exists() )
            {
                if( file.open(QIODevice::ReadOnly))
                {
                    qDebug() << "reading bytes";

                    file_bytes = QByteArray(file.readAll());
                    file.close();

                    qDebug() << "closing device";

                    /* set url for put request */
                    urlookup_->setUrl( base_url_ + "brmin-share/" + file_name_discovered );

                    /* print url */
                    qDebug() << "Storage::store() - Putting file: " << urlookup_->url();

                    /* put file to storage server */
                    manager_->put( QNetworkRequest( *urlookup_ ), file_bytes );
                }
                else
                {
                    qDebug() << "Storage::store() - Could not open file: " << file_name_discovered;
                }
            }
            else
            {
                qDebug() << "Storage::store() - Could not find file: "  << file_name_discovered;
            }
        }
        else
        {
            qDebug() << "Storage::store() - Invalid file: "  << file_name_discovered \
                     << " to send! Searching..." ;
        }
    }

}

bool Storage::fetch()
{
    /* assert file list is populated */
    if( !list_of_files_.isEmpty() )
    {
        /* fetch single file from server */
        if( file_index_ < list_of_files_.size() && file_index_ >= 0 )
        {
            QString file_name = list_of_files_.at(file_index_);
            qDebug() << "File Extracted: " << file_name;
            current_file_ = file_name;

            /* set url for putting files */
            urlookup_->setUrl( base_url_ + "brmin-share/" + file_name );

            /* print url */
            qDebug() << "Putting file: " << urlookup_->url();

            /* get file from storage server */
            manager_->get( QNetworkRequest( *urlookup_ ) );

            /* increment file counter */
            file_index_++;
        }
        else if( file_index_ < 0 )
        {
            qDebug() << "ftpControlChannel::onFinished() - File Index counter is not valid";
            file_index_ = 0;
            return false;
        }
        else
        {
            qDebug() << "ftpControlChannel::onFinished() - File get-requests completed";
            file_index_ = 0;
            return false;
        }
    }
    else
    {
        qDebug() << "Storage::fetch() - No recorded files found to fetch";
        return false;
    }
    return true;
}

void Storage::connectToShare(const QString &server)
{
    /* set url */
    base_url_ = "ftp://mds-admin:I1by&UoEJktKD^$I@" + server +":21/";

    /* connect to host via FTP service on port 21 */
    urlookup_ = new QUrl( base_url_ );

    /* connect to share */
    manager_->connectToHost(server, 21);
}

void Storage::readFileNamesFromDB()
{
    Database sourceDB;

    QString table = "files";
    QVector<QSqlRecord> result;
    QVector<QString> select_list;
    select_list.append("*");

    /* clear lists */
    list_of_files_.clear();
    result.clear();

    if(  sourceDB.connOpen() )
    {
        if( !sourceDB.select(table, select_list, result))
        {
            qDebug() << "FtpControlChannel::readFileNamesFromDB() - Unable to read file descriptions";
        }
        else
        {
            if( !result.empty() )
            {
                for( int i = 0; i < result.size(); i++ )
                {
                    /* populate list */
                    list_of_files_.append( result.at(i).field("description").value().toString() );
                }
            }
            else
            {
                qDebug() << "FtpControlChannel::readFileNamesFromDB() - No result";
            }
        }
    }
    else
    {
        qDebug() << "FtpControlChannel::readFileNamesFromDB() - Unable to connect to DB";
    }

    sourceDB.connClosed();
}

void Storage::writeFileNamesToDB( QString fileName )
{
    Database sourceDB;

    QString table = "files";
    QVector<QString> column_list;
    column_list.append("description");

    QVector<QString> value_list;
    value_list.append(fileName);

    if(  sourceDB.connOpen() )
    {
        if( !sourceDB.insert(table, column_list, value_list ))
        {
            qDebug() << "FtpControlChannel::writeFileNamesToDB() - Unable to write file descriptions";
        }
        else
        {
            qDebug() << "FtpControlChannel::writeFileNamesToDB() - File: " << fileName << " Logged";
        }
    }
    else
    {
        qDebug() << "FtpControlChannel::writeFileNamesToDB()  - Unable to connect to DB";
    }

    sourceDB.connClosed();
}

void Storage::onFinished(QNetworkReply *reply)
{
    qDebug() << "Storage::onFinished() - Finished";

    QByteArray bytes = reply->readAll();

    if( !bytes.isEmpty() )
    {
        QString str = QString::fromUtf8(bytes.data(), bytes.size());
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        qDebug() << "FtpControlChannel::onFinished() - " << QVariant(statusCode).toString();

        if(statusCode == REDIRECT)
        {
            QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            qDebug() << "redirected from " + reply->url().toString() + " to " + newUrl.toString();
            QNetworkRequest newRequest(newUrl);
            manager_->get(newRequest);
            return;
        }
        else if( statusCode == SUCCESSFUL )
        {
            if( !current_file_.isEmpty() )
            {
                QFile file(file_path + "/" + current_file_);
                if( file.open(QIODevice::WriteOnly) )
                {
                    file.write(bytes);
                    file.close();
                }

                /* clear current file description after writing */
                current_file_.clear();
            }
            else
            {
                qDebug() << "Storage::onFinished() - Storage-Server Status: (Ready)";
            }
        }
   }

}
