#ifndef FOLDER_H
#define FOLDER_H

#include "shared.h"
#include "encrypto.h"

#include <QDir>
#include <QObject>

class Folder : public QObject
{
    Q_OBJECT
public:
    Folder();

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
     * \param passphrase
     * \param temp_del_dir
     */
    void EncryptFolder(QString inputFileName, QString outputFileName, QByteArray passphrase, QDir temp_del_dir);

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
     * \param temp_del_dir
     */
    void deleteFile(QString fileName, bool isDir, QDir temp_del_dir);

    /*!
     * \brief mergeFolders - Recursive Function!
     * \param fileName
     * \param inputFilePath
     * \param outputFilePath
     */
    void mergeFolders(QString fileName, QString inputFilePath, QString outputFilePath);


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

    /*!
     * \brief packageAllRequestedModalities
     * \param modality
     */
    void packageAllRequestedModalities(QString modality);
    /*!
     * \brief traverseDirectory
     * \param modality
     * \param temp_del_dir
     */
    void traverseDirectory(QString modality , QDir temp_del_dir);
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

private:

    Encrypto encryptor_;
    QString file_path_;

};

#endif // FOLDER_H
