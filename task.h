#ifndef TASK_H
#define TASK_H

#include <QDir>
#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <encrypto.h>

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
     * \brief setPassPhrase
     * \param passphrase
     */
    void setPassPhrase(QString passphrase);
    /*!
     * \brief setSocketInput
     * \param socketNumber
     */
    void setSocketInput(int socketNumber);
    /*!
     * \brief mergeFolders
     * \param inputFolderPath
     * \param existingFolderPath
     */
    void mergeFolders(QString inputFolderPath, QString existingFolderPath);
    /*!
     * \brief DecryptFolder
     * \param fileName
     * \param repoFile
     * \return
     */
    QString DecryptFolder(QString fileName, bool repoFile);

    /*!
     * \brief EncryptFolder
     * \param inputFileName
     * \param outputFileName
     */
    void EncryptFolder(QString inputFileName, QString outputFileName);

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
    Encrypto encryptor_;
    QString passphrase_;

};

#endif // TASK_H
