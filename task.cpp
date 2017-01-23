#include <QDir>
#include "task.h"
#include "shared.h"
#include <JlCompress.h>
#include <QDirIterator>

Task::Task()
{
    // set the paths for temporary directories and repo directory
    temp_dir_.setPath(temp_path_global);
    repo_dir_.setPath(output_path_global);
}

void Task::run()
{
    if( !auto_retrieve_ )
        this->processData();
    else
        this->retrieveBiometricData();
}

bool Task::autoRetrieve() const
{
    return auto_retrieve_;
}

void Task::setAutoRetrieve( QStringList &requestedModalitiesList, \
                            bool autoRetrieve )
{
    auto_retrieve_ = autoRetrieve;
    requested_modalities_list_ = requestedModalitiesList;
}


void Task::setSocketInput(int socketNumber)
{
    // set socket number (Client Id)
    socket_number_ = socketNumber;
}

// Recursive function
void Task::mergeFolders(QString fileName,\
                        QString inputFilePath,\
                        QString outputFilePath)
{
    QDir input_dir;

    QString temp_file_name = "";
    QFileInfo temp_input_file_info(inputFilePath   +"/" + fileName);
    QFileInfo temp_output_file_info(outputFilePath +"/" + fileName);

    // check if directory
    if(temp_input_file_info.isDir())
    {
        // - set path to new directory
        input_dir.setPath(inputFilePath   +"/" + fileName);

        // - get listing of all files and dirs in directory
        QStringList input_list = input_dir.entryList();

        // - loop through all in entry list
        for(int i = 0; i < input_list.size(); i++)
        {
            // -- set file name
            temp_file_name = input_list.at(i);

            // -- skip OS file system "Dot and DotDot
            if(temp_file_name == "." || temp_file_name == ".." )
                continue;
            else
            {
                // --- check if exists within repo
                if(temp_output_file_info.exists())
                {
                    // ---- recursive call to merge folders
                    mergeFolders(temp_file_name, \
                                      (inputFilePath  + "/" + fileName),\
                                      (outputFilePath + "/" + fileName));
                }
                else if((!temp_output_file_info.exists()) || temp_input_file_info.isFile())
                {
                    // ---- move folder to repo folder
                    input_dir.rename((inputFilePath + "/" + fileName),
                                     (outputFilePath+ "/" + fileName));
                }
            }
        }
    }
    else if(temp_input_file_info.isFile())
    {
        // - check if directory with same heading exists in repo folder
        if(temp_output_file_info.exists())
        {
            // -- skip - cannot merge file, only directories
        }
        else
        {
            // -- move file to repo folder
            input_dir.rename((inputFilePath + "/" + fileName),
                             (outputFilePath+ "/" + fileName));
        }
    }
}


void Task::CompressDir(QString zipFile, QString directory)
{
    // call compress directory of folder
    if(JlCompress::compressDir(zipFile, directory))
    {
        qDebug() << "Created" << zipFile;
    }
    else
    {
        qDebug() << "Could not create" << zipFile;
    }
}

void Task::DecompressDir(QString zipFile, QString directory)
{
    // decompress file into folder
    QStringList list = JlCompress::extractDir(zipFile, directory);
}

QString Task::DecryptFolder(QString fileName, bool repoFile, QByteArray passphrase)
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

void Task::EncryptFolder(QString inputFileName,\
                         QString outputFileName,\
                         QByteArray passphrase)
{
    // read in encrypted file
    QByteArray file = encryptor_.readFile(inputFileName);

    // load passphrase
    QByteArray encrypted_file = encryptor_.encryptAES(passphrase, file, false);

    // write decrypted file
    encryptor_.writeFile(outputFileName, encrypted_file);
}

void Task::deleteFile(QString fileName, bool isDir)
{
    if(isDir)
    {
        temp_del_dir_.setPath(fileName);
        temp_del_dir_.removeRecursively();
    }
    else
    {
        temp_del_dir_.remove(fileName);
    }
}

void Task::retrieveBiometricData()
{
    // assert Populated(requested_modalities_list_)
    if( !requested_modalities_list_.isEmpty() )
    {
        for( int i = 0; i < requested_modalities_list_.size(); i++ )
        {
            traverseDirectory( requested_modalities_list_.at(i) );
        }
    }
}

void Task::traverseDirectory( QString modality )
{
    QString temp_path;
    QString sub_dir_temp_path;
    QString repo_decrypt_file_name;
    QString repo_decompressed_file_name;

    QDirIterator *sub_directory_it;
    QString file_path = "/home/esaith/Downloads/DATA";
    QDirIterator file_path_it( file_path, QDir::Files);
    QString requested_modalities_dir_path = file_path + "/requested_" + modality;

    QDir requested_modalities(file_path);
    requested_modalities.mkdir(requested_modalities_dir_path);

    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);



    // decrypt & decompress
    while (file_path_it.hasNext())
    {
        temp_path = file_path_it.next();

        repo_decrypt_file_name = DecryptFolder( temp_path, \
                                                true,\
                                                repo_folder_key);
        // unzip existing file
        repo_decompressed_file_name = repo_decrypt_file_name + "_decompressed";
        DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

        // delete the repo decripted files only
        deleteFile(repo_decrypt_file_name, false);

        // traverse repo decompressed files
        sub_directory_it = new QDirIterator( repo_decompressed_file_name,\
                                             ( QDir::Dirs | \
                                               QDir::NoDot | \
                                               QDir::NoDotDot | \
                                               QDir::NoDotAndDotDot \
                                             ), \
                                             QDirIterator::Subdirectories);

        qDebug() << "Files Existing are - \n";
        while( sub_directory_it->hasNext() )
        {
            sub_dir_temp_path = sub_directory_it->next();

            QStringList path_info = sub_dir_temp_path.split("/");
            qDebug() << path_info.last();

            // Copy data to its corresponding directory";
            if( path_info.last() == "Fingerprints" && modality.toInt() == FINGERPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Palmprints" && modality.toInt() == PALMPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Iris" && modality.toInt() == IRIS )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Footprints" && modality.toInt() == FOOTPRINTS )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "Ear2D" && modality.toInt() == EAR2D )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
            else if( path_info.last() == "EAR3D" && modality.toInt() == EAR3D )
            {
                copyDir( sub_dir_temp_path, \
                         requested_modalities_dir_path , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied fingerprints @, " \
                         << sub_dir_temp_path;
            }
        }
    }
}
/*
 * if(QDir("/home/highlander/Desktop/dir").entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
{
    QMessageBox::information(this,"Directory is empty","Empty!!!");
}*/
void Task::processData()
{
    QStringList new_files  = temp_dir_.entryList();

    QString temp_file_name              = "";
    QString original_file_name          = "";
    QString temp_merged_file_name       = "";
    QString new_decrypt_file_name       = "";
    QString repo_decrypt_file_name      = "";
    QString new_decompressed_file_name  = "";
    QString repo_decompressed_file_name = "";

    QFile repo_file;
    QFileInfo temp_file;

    const bool isDir = true;

    // get passphrase
    // HEREP
    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);

    // loop  through all recieved files
    for(int i = 0; i < new_files.size(); i++)
    {
        // - set file name
        temp_file_name =  new_files.at(i);

        // - skip dot and dotdot
        if(temp_file_name == "." || temp_file_name == ".." )
            continue;
        else
        {
            // -- get file name excludong client number that sent the file
            original_file_name = new_files.at(i).split("#").at(1);

            // -- check if dependency
            if(original_file_name.contains(".dependency_"))
            {
                // skip
            }
            else
            {
                // --- set file name with repo path
                repo_file.setFileName(output_path_global + original_file_name);

                // --- check if file exists in repo
                if(repo_file.exists())
                {
                    // ---- if exists
                    // ---- decrypt folder on repo
                    repo_decrypt_file_name = DecryptFolder(output_path_global + original_file_name, \
                                                           true,\
                                                           repo_folder_key);

                    // ---- unzip existing file
                    repo_decompressed_file_name = repo_decrypt_file_name+"_decompressed";
                    DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

                    // ---- decrypt new folder
                    // ---- get passphrase
                    QString dependency_file_name = new_files[i];
                    dependency_file_name.remove(".zip");
                    dependency_file_name.insert(0, temp_dir_.absolutePath() + "/");
                    dependency_file_name.append(".dependency_");
                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                     new_decrypt_file_name = DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                          false,\
                                                          temp_folder_key);

                    // ---- unzip new file
                    new_decompressed_file_name = new_decrypt_file_name+"_decompressed";
                    DecompressDir(new_decrypt_file_name, new_decompressed_file_name);

                    // ---- get basename
                    temp_file.setFile(original_file_name);

                    // ---- merge folders
                    mergeFolders(temp_file.baseName(),\
                                 new_decompressed_file_name,\
                                 repo_decompressed_file_name);

                    // ---- compress folder
                    temp_merged_file_name = repo_decompressed_file_name + "_compressed.zip";
                    CompressDir(temp_merged_file_name, repo_decompressed_file_name);

                    // ---- remove temporay files
                    deleteFile(dependency_file_name,                             !isDir);
                    deleteFile(new_decrypt_file_name,                            !isDir);
                    deleteFile(repo_decrypt_file_name,                           !isDir);
                    deleteFile(output_path_global + original_file_name,          !isDir);
                    deleteFile(temp_dir_.absolutePath() + "/" + new_files.at(i), !isDir);

                    // ---- delete temp directories
                    deleteFile(new_decompressed_file_name,  isDir);
                    deleteFile(repo_decompressed_file_name, isDir);

                    // ---- encrypt
                    EncryptFolder(temp_merged_file_name, \
                                  (output_path_global + original_file_name),\
                                  repo_folder_key);

                    // ---- delete zip file only after encryption [Others must be deleted before]
                    deleteFile(temp_merged_file_name, false);

                    // ---- send signal of completion
                    qDebug() << "Completed";
                    emit completed();
                }
                else
                {
                    // ---- decrypt temp folder using given key
                    QString dependency_file_name = new_files[i];
                    dependency_file_name.remove(".zip");
                    dependency_file_name.insert(0, temp_dir_.absolutePath() + "/");
                    dependency_file_name.append(".dependency_");

                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                    new_decrypt_file_name = DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                          false,\
                                                          temp_folder_key);


                    // encrypt folder using repo key & move to repo folder
                    EncryptFolder(new_decrypt_file_name, \
                                  (output_path_global + original_file_name),\
                                  repo_folder_key);

                    // delete files
                    deleteFile(temp_dir_.absolutePath() + "/" + new_files.at(i),        !isDir);
                    deleteFile(dependency_file_name,   !isDir);
                    deleteFile(new_decrypt_file_name,  !isDir);

                    qDebug() << "Completed";
                    emit completed();
                }
            }


        }


    }
    // free memory
    encryptor_.freeRSAKey(pvt_key);
}

bool Task::copyDir(const QString source, const QString destination, const bool override)
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

/*
 *  switch( modality.toInt() )
     {
         case IRIS:
         break;

         case FINGERPRINTS:
         break;

         case EAR2D:
         break;

         case EAR3D:
         break;

         case FOOTPRINTS:
         break;

         case PALMPRINTS:
         break;

     }
 */
