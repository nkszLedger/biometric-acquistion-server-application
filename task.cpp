#include "task.h"
#include "shared.h"

#include <QDir>
#include <QSql>
#include <QVector>
#include <QString>
#include <QSqlRecord>
#include <database.h>
#include <JlCompress.h>
#include <QDirIterator>
#include <QSqlDatabase>
#include <sharedsettings.h>
#include <QRegularExpression>

Task::Task()
{
    // initialise retrieval flag
    auto_retrieve_ = false;

    // set the paths for temporary directories and repo directory
    temp_dir_.setPath(temp_path_global);
    repo_dir_.setPath(output_path_global);

    file_path_= "/home/esaith/Downloads/DATA";
    requested_modalities_dir_path_ = file_path_ + "/requested_";
}

void Task::run()
{
    if( !auto_retrieve_  )
    {
        qDebug()<< "Is data retrieved?"<<auto_retrieve_;
        this->processData();
    }
    else
    {
        qDebug()<< "Retreiving Data:" << auto_retrieve_;
        this->retrieveBiometricData();
    }
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
        qDebug() << "Task::CompressDir() - Created: "   << zipFile \
                 << "Task::CompressDir() - File size: " << zipFile.size();
    }
    else
    {
        qDebug() << "Task::CompressDir() - Could not create" << zipFile;
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

    // delete input file
    deleteFile(inputFileName, false);

    // write encrypted file
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
    // traverse directory -  decrypts, decompresses and
    // collects data to store modality corresponding 'requested' directories
    if( !requested_modalities_list_.isEmpty() )
    {
        for( int i = 0; i < requested_modalities_list_.size(); i++ )
        {
            traverseDirectory( requested_modalities_list_.at(i) );
            // Check for empty 'requested data directories and remove
            // Combine folders into one directory
            packageAllRequestedModalities(requested_modalities_list_.at(i));
        }
    }

    CompressDir(file_path_ + "/retrievedModalityData.zip", file_path_ + "/requested_mods_combined");

    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = "MDSBRMP";// encryptor_.decryptRSA(pvt_key, repo_folder_data);

    EncryptFolder(file_path_ +"/retrievedModalityData.zip", \
                    file_path_ + "/retrievedModalityData.zip",
                    repo_folder_key);

    emit requestedModalitiesReady(file_path_ + "/retrievedModalityData.zip");

    // clean up directory
    //QRegularExpression regex("*/_decompressed");
    deleteFile(file_path_ + "/requested_mods_combined", true);
    deleteFile(file_path_ + "/retrievedModalityData.zip", false);

    encryptor_.freeRSAKey(pvt_key);

}
void Task::packageAllRequestedModalities( QString modality )
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

void Task::traverseDirectory( QString modality )
{
    QString temp_path;
    unsigned int counter = 1;
    QString sub_dir_temp_path;
    QString repo_decrypt_file_name;
    QString repo_decompressed_file_name;

    QDirIterator *sub_directory_it;
    QString temp_requested_modalities_dir_path = file_path_ + "/" + getModalityName(modality);
    QDirIterator file_path_it( file_path_, QDir::Files);

    QDir requested_modalities(file_path_);
    requested_modalities.mkdir(temp_requested_modalities_dir_path);

    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);

    encryptor_.freeRSAKey(pvt_key);

    // decrypt & decompress
    while (file_path_it.hasNext())
    {
        temp_path = file_path_it.next();
        QStringList temp_path_info = temp_path.split("/");
        QString participant_id = temp_path_info.last().split(".zip").first();

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
            else if( path_info.last() == "EAR3D" && modality.toInt() == EAR3D )
            {
                copyDir( sub_dir_temp_path, \
                         new_modality_dir_for_participant  , \
                         false);
                qDebug() << "Task::traverseDirectory() - Copied ear3D @, " \
                         << sub_dir_temp_path;
            }
        }
        counter++;
    }

    // free memory
    delete sub_directory_it;
}

QString Task::getModalityName(QString modality)
{
    switch( modality.toInt())
    {
        case EAR2D          : return "Ear2D";
        break;
        case EAR3D          : return "Ear3D";
        break;
        case FOOTPRINTS     : return "Footprints";
        break;
        case PALMPRINTS     : return "Palmprints";
        break;
        case FINGERPRINTS   : return "Fingerprints";
        break;
    }
}


QString Task::getGenderAndDOB( QString participant_id )
{
    Database *sourceDB;
    sourceDB = new Database();

    unsigned int first_index = 0;
    QString dob;
    QString gender;
    QString table = "participant";
    QVector<QString> column_list;
    column_list.append("hashed_id");

    QVector<QString> value_list;
    value_list.append(participant_id);

    QVector<QString> select_list;
    select_list.append("gender");
    select_list.append("date_of_birth" );

    QVector<QSqlRecord> result;
    result.clear();

    if(  sourceDB->connOpen() )
    {
        if( !sourceDB->select( table, select_list, column_list, value_list, result ) )
        {
            qDebug() << "Task::getGenderAndDOB() - Unable to query DB";
            return "";
        }
        else
        {
            if( result.isEmpty() )
            {
                qDebug() << "Task::getGenderAndDOB() - No data found";
                return "";
            }
            else
            {
                gender = result.at(first_index).field("gender").value().toString();
                dob    = result.at(first_index).field("date_of_birth").value().toString();
            }
        }
    }
    else
    {
        qDebug() << "Task::getGenderAndDOB() - Unable to connect to DB";
        return "";
    }

    sourceDB->connClosed();
    delete sourceDB;

    return gender + "#" +dob;

}


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
        qDebug()<<" Temp FileName: "<<temp_file_name;

        // - skip dot and dotdot
        if(temp_file_name == "." || temp_file_name == ".." )
            continue;
        else
        {
            // -- get file name excludong client number that sent the file
            original_file_name = new_files.at(i).split("#").at(1);
            qDebug()<<" original_file_name FileName: "<< original_file_name;

            // -- check if dependency
            if(original_file_name.contains(".dependency_"))
            {
                // skip
            }
            else
            {
                // --- set file name with repo path
                repo_file.setFileName(output_path_global + original_file_name);

                qDebug()<<"set file name with repo path: "<<output_path_global + original_file_name;
                // --- check if file exists in repo
                if(repo_file.exists())
                {

                    // ---- if exists
                    // ---- decrypt folder on repo
                    repo_decrypt_file_name = DecryptFolder(output_path_global + original_file_name, \
                                                           true,\
                                                           repo_folder_key);
                    qDebug()<<"decrypt folder on repo: "<<output_path_global + original_file_name;

                    // ---- unzip existing file
                    repo_decompressed_file_name = repo_decrypt_file_name+"_decompressed";
                    DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

                    qDebug()<<"Uzipped Existing file name: "<<repo_decompressed_file_name;
                    // ---- decrypt new folder
                    // ---- get passphrase
                    QString dependency_file_name = new_files[i];
                    dependency_file_name.remove(".zip");
                    dependency_file_name.insert(0, temp_dir_.absolutePath() + "/");
                    dependency_file_name.append(".dependency_");
                    qDebug()<<"Dependency_file_name: "<<dependency_file_name;

                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                     new_decrypt_file_name = DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                          false,\
                                                          temp_folder_key);
                    qDebug()<<"New Decrypt File Name: "<<dependency_file_name ;
                    // ---- unzip new file
                    new_decompressed_file_name = new_decrypt_file_name+"_decompressed";
                    qDebug()<<"New Decompressed File Name: "<<new_decompressed_file_name;
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

                    qDebug()<<"Temp Merged_File Name: "<<temp_merged_file_name;
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
                    qDebug()<<"dependency_file_name: "<<dependency_file_name;

                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                    new_decrypt_file_name = DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                          false,\
                                                          temp_folder_key);

                    qDebug()<<"new_decrypt_file_name: "<<new_decrypt_file_name;
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
