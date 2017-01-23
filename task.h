#ifndef TASK_H
#define TASK_H

#include <QDir>
#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <encrypto.h>
#include <QStringList>

class Task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Task();

    /*!
     * \brief processData
     */
    void processData();

    /*!
     * \brief setSocketInput
     * \param socketNumber
     */
    void setSocketInput(int socketNumber);
    /*!
     * \brief DecryptFolder
     * \param fileName
     * \param repoFile
     * \return
     */
    QString DecryptFolder(QString fileName, bool repoFile, QByteArray passphrase);

    /*!
     * \brief EncryptFolder
     * \param inputFileName
     * \param outputFileName
     */
    void EncryptFolder(QString inputFileName, QString outputFileName, QByteArray passphrase);

    /*!
     * \brief CompressDir
     * \param zipFile
     * \param directory
     */
    void CompressDir(QString zipFile, QString directory);

    /*!
     * \brief DecompressDir
     * \param zipFile
     * \param directory
     */
    void DecompressDir(QString zipFile, QString directory);

    /*!
     * \brief deleteFile
     * \param fileName
     * \param isDir
     */
    void deleteFile(QString fileName, bool isDir);

    /*!
     * \brief auto_retrieve
     * \return
     */
    bool autoRetrieve() const;

    /*!
     * \brief setAutoRetrieve
     * \param requested_modalities_list
     * \param autoRetrieve
     */
    void setAutoRetrieve(QStringList &requestedModalitiesList, \
                          bool autoRetrieve);
    /*!
     * \brief mergeFolders - Recursive Function!
     * \param fileName
     * \param inputFilePath
     * \param outputFilePath
     */
    void mergeFolders(QString fileName, QString inputFilePath, QString outputFilePath);



signals:
    /*!
     * \brief completed
     */
    void completed();

public slots:

protected:
    void run();

private:
    QDir repo_dir_;
    QDir temp_dir_;
    QDir temp_del_dir_;
    int socket_number_;
    bool auto_retrieve_;
    Encrypto encryptor_;
    QStringList requested_modalities_list_;

    /*!
     * \brief getRequesteModalities
     */
    void getRequesteModalities();
    /*!
     * \brief retrieveBiometricData
     */
    void retrieveBiometricData();
    /*!
     * \brief traverseDirectory
     * \param modality
     */
    void traverseDirectory( QString modality );

    bool copyDir(const QString source, const QString destination, const bool override);
};

#endif // TASK_H
