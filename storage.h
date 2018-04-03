#ifndef STORAGE_H
#define STORAGE_H

#include "shared.h"
#include "folder.h"

#include <QUrl>
#include <QList>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class Storage : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Storage
     */
    explicit Storage(QObject *parent = nullptr);

    /* store single fata zip file to FTP share */
    void store(QString filePath);

    /* fetch single data zip file from FTP share */
    bool fetch();

    /* Connect to an FTP share */
    void connectToShare(const QString &server);

public slots:

    /* reply after connecting to share */
    void onFinished(QNetworkReply* reply);


private:

    QUrl *urlookup_;
    Folder data_folder_;
    QNetworkAccessManager *manager_;

    int file_index_;
    QString base_url_;
    QString current_file_;
    QList<QString> list_of_files_;

    /* helper functions */
    void readFileNamesFromDB();
    void writeFileNamesToDB(QString fileName);
};

#endif // STORAGE_H
