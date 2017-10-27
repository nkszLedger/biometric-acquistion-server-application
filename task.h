#ifndef TASK_H
#define TASK_H

#include "shared.h"

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
    /*!
     * \brief requestedModalitiesReady
     * \param requestedModalitiesfilePath
     */
    void requestedModalitiesReady(QString requestedModalitiesfilePath);

public slots:

protected:
    void run();

private:
    QDir repo_dir_;
    QDir temp_dir_;
    QDir temp_del_dir_;
    int socket_number_;
    QString file_path_;
    bool auto_retrieve_;
    Encrypto encryptor_;
    QStringList requested_modalities_list_;
    QString requested_modalities_dir_path_;

    /*!
     * \brief getModalityName
     * \param modality
     * \return
     */
    QString getModalityName(QString modalityNumber);
    /*!
     * \brief getRequesteModalities
     */
    void getRequesteModalities();
    /*!
     * \brief retrieveBiometricData
     */
    void retrieveBiometricData();

    /*!
     * \brief packageAllRequestedModalities
     * \param modality
     */
    void packageAllRequestedModalities(QString modality);
    /*!
     * \brief traverseDirectory
     * \param modality
     */
    void traverseDirectory( QString modality );
    /*!
     * \brief copyDir
     * \param source
     * \param destination
     * \param override
     * \return
     */
    bool copyDir(const QString source, const QString destination, const bool override);

    /*!
     * \brief getGenderAndDOB
     * \param participant_id
     * \return
     */
    QString getGenderAndDOB(QString participant_id);

    /*!
     * \brief mergeFoldersForModality
     * \param modalityNumber
     * \param inputFilePath
     * \param outputFilePath
     */
    void mergeFoldersForModality(QString modalityNumber, \
                                        QString inputFilePath, \
                                        QString outputFilePath);

    /*!
     * \brief mergeAllExistingBiometrics
     * \param new_decompressed_file_name
     * \param repo_decompressed_file_name
     */
    void mergeAllExistingModalities(QString new_decompressed_file_name, QString repo_decompressed_file_name);
};

#endif // TASK_H
