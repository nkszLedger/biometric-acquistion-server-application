#ifndef CLIENT_H
#define CLIENT_H

#include <task.h>
#include <QFile>
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
    /*!
     * \brief Client
     * \param parent
     */
    explicit Client(QObject *parent = 0);

    /*!
     * \brief SetSocket
     * \param socketDescriptor
     */
    void SetSocket(qintptr socketDescriptor);

    /*!
     * \brief getHeader
     * \param inLine
     */
    void getHeader(QString inLine);

    /*!
     * \brief getEncryptedFiles
     * \param data
     */
    void getEncryptedFiles(QByteArray data);

    /*!
     * \brief setFileName
     * \param index
     */
    void setFileName(int index);

    /*!
     * \brief notifyClientOnClose
     */
    void notifyClientOnClose();

signals:
    /*!
     * \brief file_size_signal
     */
    void file_size_signal();

public slots:
    /*!
     * \brief connected
     */
    void connected();
    /*!
     * \brief disconnected
     */
    void disconnected();
    /*!
     * \brief readyRead
     */
    void readyRead();

    /*!
     * \brief TaskResult
     */
    void TaskResult();
    /*!
     * \brief sendEncryptedFile
     * \param requestedModalitiesFilePath
     */
    void sendEncryptedFile(QString requestedModalitiesFilePath);

private:

    QString current_file_name_;

    QStringList file_names_;
    QStringList file_sizes_;

    QTcpSocket *socket;
    QFile *requested_biometrics_file_;

    int no_files_;
    int size_counter_;
    int file_size_counter_;
    int current_file_index_;

    /*!
     * \brief retrieveRequestedBioModalities
     * \param in_line
     */
    void retrieveRequestedBioModalities(QString in_line);
};

#endif // CLIENT_H
