#include "folder.h"

Folder::Folder()
{
    file_path_ = data_file_path;
}

void Folder::mergeFolders(QString fileName,\
                        QString inputFilePath,\
                        QString outputFilePath)
{
    qDebug() << "mergeFolders() - merging process...";
    qDebug() << "mergeFolders() - " << fileName;
    qDebug() << "mergeFolders() - " << inputFilePath   + "/" + fileName;
    qDebug() << "mergeFolders() - " << outputFilePath  + "/" + fileName;
    qDebug() << "**********************************************************************************";

    QDir input_dir;

    QString temp_file_name = "";
    QFileInfo temp_input_file_info(inputFilePath   +"/" + fileName);
    QFileInfo temp_output_file_info(outputFilePath +"/" + fileName);

    /* check if directory */
    if(temp_input_file_info.isDir())
    {

        qDebug() << "mergeFolders() - " << temp_input_file_info.baseName() << " is directory";

        /* set path to new directory */
        input_dir.setPath(inputFilePath   +"/" + fileName);

        /* get listing of all files and dirs in directory */
        QStringList input_list = input_dir.entryList();

        /* loop through all in entry list */
        for(int i = 0; i < input_list.size(); i++)
        {
            /* set file name */
            temp_file_name = input_list.at(i);

            /* skip OS file system "Dot and DotDot */
            if(temp_file_name == "." || temp_file_name == ".." )
                continue;
            else
            {
                /* check if exists within repo */
                if(temp_output_file_info.exists())
                {
                    /* recursive call to merge folders */
                    mergeFolders(temp_file_name, \
                                      (inputFilePath  + "/" + fileName),\
                                      (outputFilePath + "/" + fileName));
                }
                else if((!temp_output_file_info.exists()) || temp_input_file_info.isFile())
                {
                    /* move folder to repo folder */
                    input_dir.rename((inputFilePath + "/" + fileName),
                                     (outputFilePath+ "/" + fileName));
                }
            }
        }
    }
    else if(temp_input_file_info.isFile())
    {
        qDebug() << "mergeFolders() - " << temp_input_file_info.baseName() << " is file";

        /* check if directory with same heading exists in repo folder */
        if(temp_output_file_info.exists())
        {
            /* skip - cannot merge file, only directories */
        }
        else
        {
            /* move file to repo folder */
            input_dir.rename((inputFilePath + "/" + fileName),
                             (outputFilePath+ "/" + fileName));
            qDebug() << "mergeFolders() - renamed";

        }
    }
    else
    {
        qDebug() << "mergeFolders() - Could not merge files or directories";
    }
}


void Folder::CompressDir(QString zipFile, QString directory)
{
    // call compress directory of folder
    if(JlCompress::compressDir(zipFile, directory))
    {
        qDebug() << "Task::CompressDir() - Created: "   << zipFile \
                 << "Task::CompressDir() - File size: " << zipFile.size();
    }
    else
    {
        qDebug() << "Task::CompressDir() - Could not create" << zipFile;
    }
}

void Folder::DecompressDir(QString zipFile, QString directory)
{
    // decompress file into folder
    QStringList list = JlCompress::extractDir(zipFile, directory);
}

QString Folder::DecryptFolder(QString fileName, \
                                bool repoFile, \
                                QByteArray passphrase)
{
    // read in encrypted file
    QByteArray encrypted_file = encryptor_.readFile(fileName);

    //load passphrase
    QByteArray decrypted_file = encryptor_.decryptAES(passphrase, encrypted_file);

    // new File name for decrypted folder
    QString new_file_name;
    if(repoFile)
        new_file_name = fileName + "_repo_temp.zip";
    else
        new_file_name = fileName + "_new_temp.zip";

    //write decrypted file
    encryptor_.writeFile(new_file_name, decrypted_file);

    // return decrypted file path
    return new_file_name;
}

void Folder::EncryptFolder(QString inputFileName,\
                                QString outputFileName,\
                                QByteArray passphrase,  \
                                QDir temp_del_dir)
{
    // read in encrypted file
    QByteArray file = encryptor_.readFile(inputFileName);

    // load passphrase
    QByteArray encrypted_file = encryptor_.encryptAES(passphrase, file, false);

    // delete input file
    deleteFile(inputFileName, false, temp_del_dir);

    // write encrypted file
    encryptor_.writeFile(outputFileName, encrypted_file);
}

void Folder::deleteFile(QString fileName, bool isDir, QDir temp_del_dir)
{
    if(isDir)
    {
        temp_del_dir.setPath(fileName);
        temp_del_dir.removeRecursively();
    }
    else
    {
        temp_del_dir.remove(fileName);
    }
}

void Folder::packageAllRequestedModalities( QString modality )
{
    QDirIterator *file_path_it = new QDirIterator( file_path_, \
                                                   QStringList() << getModalityName(modality));

    QDir requested_modalities(file_path_);
    requested_modalities.mkdir(file_path_ + "/requested_mods_combined");

    while( file_path_it->hasNext() )
    {
        QDir dir;

        if( !dir.rename( file_path_it->next(), \
                         file_path_ + "/requested_mods_combined/" + getModalityName(modality)))
        {
            qDebug() << file_path_ + "/requested_mods_combined/" + getModalityName(modality) \
                     << " - Movement failed??";
        }
    }

    // free memory
    delete file_path_it;
}

void Folder::traverseDirectory( QString modality,  QDir temp_del_dir)
{
    qDebug() << "Modality: " << modality;

    QString temp_path;
    unsigned int counter = 1;
    QString sub_dir_temp_path;
    QString repo_decrypt_file_name;
    QString repo_decompressed_file_name;

    qDebug() << "Modality Name: " << getModalityName(modality);
    QString temp_requested_modalities_dir_path = file_path_ + "/" + getModalityName(modality);
    qDebug() << "temp_requested_modalities_dir_path: " << temp_requested_modalities_dir_path;

    QDirIterator file_path_it( file_path_, QDir::Files);

    QDir requested_modalities(file_path_);
    requested_modalities.mkdir(temp_requested_modalities_dir_path);

    qDebug() << "Reading keys... ";
    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);

    encryptor_.freeRSAKey(pvt_key);

    QDirIterator *sub_directory_it;
    qDebug() << "Going to decrypt & decompress...";
    // decrypt & decompress
    while (file_path_it.hasNext())
    {
        temp_path = file_path_it.next();
        QStringList temp_path_info = temp_path.split("/");

        /*! TODO
         *
         */
        QString participant_id = temp_path_info.last().split("-").first();

        qDebug() << "Participant ID: " << participant_id;

        repo_decrypt_file_name = DecryptFolder( temp_path, \
                                                true,\
                                                repo_folder_key);
        // unzip existing file
        repo_decompressed_file_name = repo_decrypt_file_name + "_decompressed";
        DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

        // delete the repo decripted files only
        deleteFile(repo_decrypt_file_name, false, temp_del_dir);

        // traverse repo decompressed files
        sub_directory_it = new QDirIterator( repo_decompressed_file_name,\
                                             ( QDir::Dirs | \
                                               QDir::NoDot | \
                                               QDir::NoDotDot | \
                                               QDir::NoDotAndDotDot \
                                             ), \
                                             QDirIterator::Subdirectories);

        // create directory for requested modality from participant
        QDir modality_n( temp_requested_modalities_dir_path );
        QString new_modality_dir_for_participant = temp_requested_modalities_dir_path + "/" \
                                                    + getModalityName(modality) +"_"+ \
                                                        QString::number(counter) + \
                                                            getGenderAndDOB( participant_id );

        modality_n.mkdir( new_modality_dir_for_participant );

        while( sub_directory_it->hasNext() )
        {
            sub_dir_temp_path = sub_directory_it->next();

            QStringList path_info = sub_dir_temp_path.split("/");
            qDebug() << path_info.last();

            // Copy data to its corresponding directory";
            if( path_info.last() == "Fingerprints" && modality.toInt() == FINGERPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant, \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Palmprints" && modality.toInt() == PALMPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied palmprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Iris" && modality.toInt() == IRIS )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied iris @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Footprints" && modality.toInt() == FOOTPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied footprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Ear2D" && modality.toInt() == EAR2D )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied ear2D @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Ear3D" && modality.toInt() == EAR3D )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied ear3D @, " \
                         << sub_dir_temp_path;
            }
            // Copy data to its corresponding directory";
            if( (path_info.last() == "Microscope") && \
                    modality.toInt() == MICROSCOPE )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant, \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied microscope @, " \
                         << sub_dir_temp_path;
            }
        }
        counter++;
    }

    // free memory
    delete sub_directory_it;
}

void Folder::mergeFoldersForModality(QString modalityNumber, \
                                        QString inputFilePath,\
                                        QString outputFilePath)
{
    QString expected_modality_folder = getModalityName(modalityNumber) ;

    QDir dir( inputFilePath + "/" + expected_modality_folder );

    if( dir.exists() )
    {
        mergeFolders(expected_modality_folder,\
                        inputFilePath,\
                        outputFilePath);

    }
    else
    {
        qDebug() << "Task::mergeFoldersForModality() - Could not find folder: " << expected_modality_folder;
    }
}

void Folder::mergeAllExistingModalities( QString new_decompressed_file_name,\
                                            QString repo_decompressed_file_name)
{
    //**********************************************************
    // ---- get basename
    //temp_file.setFile(target_file_name);

    // ---- merge folders
    /*mergeFolders(temp_file.baseName(),\
                 new_decompressed_file_name,\
                 repo_decompressed_file_name);*/

    //**********************************************************

    mergeFoldersForModality(QString::number(IRIS), \
                                    new_decompressed_file_name, \
                                    repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(FINGERPRINTS), \
                                    new_decompressed_file_name,\
                                    repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(EAR2D), \
                                    new_decompressed_file_name, \
                                    repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(EAR3D), \
                                    new_decompressed_file_name, \
                                    repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(FOOTPRINTS), \
                                    new_decompressed_file_name, \
                                        repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(PALMPRINTS), \
                                    new_decompressed_file_name, \
                                    repo_decompressed_file_name );

    mergeFoldersForModality(QString::number(MICROSCOPE), \
                                    new_decompressed_file_name,
                                    repo_decompressed_file_name );

}

bool Folder::copyDir(const QString source, \
                        const QString destination, \
                        const bool override)
{
    QDir directory(source);
    bool error = false;

    if (!directory.exists()) {
        return false;
    }

    QStringList dirs = directory.entryList(QDir::AllDirs | QDir::Hidden);
    QStringList files = directory.entryList(QDir::Files | QDir::Hidden);

    QList<QString>::iterator d,f;

    for (d = dirs.begin(); d != dirs.end(); ++d) {
        if ((*d) == "." || (*d) == "..") {
            continue;
        }

        if (!QFileInfo(directory.path() + "/" + (*d)).isDir()) {
            continue;
        }

        QDir temp(destination + "/" + (*d));
        temp.mkpath(temp.path());

        if (!copyDir(directory.path() + "/" + (*d), destination + "/" + (*d), override)) {
            error = true;
        }
    }

    for (f = files.begin(); f != files.end(); ++f) {
        QFile tempFile(directory.path() + "/" + (*f));


        if (QFileInfo(directory.path() + "/" + (*f)).isDir()) {
            continue;
        }

        QFile destFile(destination + "/" + directory.relativeFilePath(tempFile.fileName()));

        if (destFile.exists() && override) {
            destFile.remove();
        }

        if (!tempFile.copy(destination + "/" + directory.relativeFilePath(tempFile.fileName()))) {
            error = true;

        }
    }
    return !error;
}
