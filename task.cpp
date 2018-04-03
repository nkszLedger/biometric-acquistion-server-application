#include "task.h"

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
#include <QRegularExpressionMatch>

Task::Task()
{
    /* initialise retrieval flag */
    auto_retrieve_ = false;

    /* set the paths for temporary directories and repo directory */
    temp_dir_.setPath(temp_path_global);
    repo_dir_.setPath(output_path_global);
    storage_.connectToShare(storage_ip_address);

    file_path_= data_file_path;
    requested_modalities_dir_path_ = file_path_ + "/requested_";
}

void Task::run()
{
        if( !auto_retrieve_  )
        {
            /* process and store data */
            qDebug()<< "Is data retrieved? "<< auto_retrieve_;
            this->processData();
        }
        else
        {
            /* fetch data from share and mount to host */
            qDebug()<< "Retreiving Data:"  << auto_retrieve_;
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
    /* set socket number (Client Id) */
    socket_number_ = socketNumber;
}

void Task::processData()
{
    QStringList new_files  = temp_dir_.entryList();

    QString temp_file_name              = "";
    QString target_file_name            = "";
    QString original_file_name          = "";
    QString temp_merged_file_name       = "";
    QString new_decrypt_file_name       = "";
    QString repo_decrypt_file_name      = "";
    QString new_decompressed_file_name  = "";
    QString repo_decompressed_file_name = "";

    QFile repo_file;
    QFileInfo temp_file;

    const bool isDir = true;

    /* get passphrase */
    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);

    /* loop  through all recieved files */
    for(int i = 0; i < new_files.size(); i++)
    {
        /* set file name */
        temp_file_name =  new_files.at(i);
        qDebug()<<" Temp FileName: "<<temp_file_name;

        /* skip dot and dotdot */
        if(temp_file_name == "." || temp_file_name == ".." )
            continue;
        else
        {
            /* get file name excluding client number that sent the file */
            target_file_name   = getModalityName( new_files.at(i).split("#").at(0) );
            original_file_name = new_files.at(i).split("#").at(1);

            qDebug()<<" target_file_name FileName: "<< target_file_name;
            qDebug()<<" original_file_name FileName: "<< original_file_name;

            /* check if dependency */
            if(original_file_name.contains(".dependency_"))
            {
                /* skip */
            }

            else
            {
                /* set file name with repo path */
                repo_file.setFileName(output_path_global + original_file_name);

                qDebug() << "set file name with repo path: " << output_path_global + original_file_name;

                /* check if file exists in repo */
                if(repo_file.exists())
                {
                    qDebug() << "Repo: " << output_path_global + original_file_name << ", exists";

                    /* if exists & decrypt folder on repo */
                    repo_decrypt_file_name = data_folder_.DecryptFolder(output_path_global + original_file_name, \
                                                                            true,\
                                                                            repo_folder_key);

                    qDebug() << "decrypt folder on repo: " << output_path_global + original_file_name;

                    /* unzip existing file */
                    repo_decompressed_file_name = repo_decrypt_file_name+"_decompressed";
                    data_folder_.DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

                    qDebug() << "Uzipped Existing file name: " << repo_decompressed_file_name;

                    /* decrypt new folder & get passphrase */
                    QString dependency_file_name = new_files[i];
                    dependency_file_name.remove(".zip");
                    dependency_file_name.insert(0, temp_dir_.absolutePath() + "/");
                    dependency_file_name.append(".dependency_");

                    qDebug() << "Dependency_file_name: " << dependency_file_name;

                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key  = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                     new_decrypt_file_name = data_folder_.DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                          false,\
                                                          temp_folder_key);

                     qDebug() << "New Decrypt File Name: " << dependency_file_name ;

                    /* unzip new file */
                    new_decompressed_file_name = new_decrypt_file_name+"_decompressed";

                    qDebug() << "New Decompressed File Name: " << new_decompressed_file_name;
                    data_folder_.DecompressDir(new_decrypt_file_name, new_decompressed_file_name);

                    /* ************* Crucial for merging existing folders **************** */

                    data_folder_.mergeAllExistingModalities(new_decompressed_file_name, repo_decompressed_file_name );

                    //**********************************************************

                    /* compress folder */
                    temp_merged_file_name = repo_decompressed_file_name + "_compressed.zip";
                    data_folder_.CompressDir(temp_merged_file_name, repo_decompressed_file_name);

                    qDebug() << "Temp Merged_File Name: " << temp_merged_file_name;

                    /* remove temporay files */
                    data_folder_.deleteFile(dependency_file_name,                             !isDir, temp_del_dir_);
                    data_folder_.deleteFile(new_decrypt_file_name,                            !isDir, temp_del_dir_);
                    data_folder_.deleteFile(repo_decrypt_file_name,                           !isDir, temp_del_dir_);
                    data_folder_.deleteFile(output_path_global + original_file_name,          !isDir, temp_del_dir_);
                    data_folder_.deleteFile(temp_dir_.absolutePath() + "/" + new_files.at(i), !isDir, temp_del_dir_);

                    /* delete temp directories */
                    data_folder_.deleteFile(new_decompressed_file_name,  isDir, temp_del_dir_);
                    data_folder_.deleteFile(repo_decompressed_file_name, isDir, temp_del_dir_);

                    /* encrypt */
                    data_folder_.EncryptFolder( temp_merged_file_name, \
                                                (output_path_global + original_file_name),\
                                                 repo_folder_key, temp_del_dir_);

                    /* delete zip file only after encryption [Others must be deleted before] */
                    data_folder_.deleteFile(temp_merged_file_name, false, temp_del_dir_);

                    /* log received file to db */
                    qDebug() << "logged received file name: " << original_file_name;
                    storage_.writeFileNamesToDB( original_file_name );

                    /* send signal of completion */
                    qDebug() << "Completed";
                    emit completed();
                }
                else
                {
                    /* decrypt temp folder using given key */
                    QString dependency_file_name = new_files[i];
                    dependency_file_name.remove(".zip");
                    dependency_file_name.insert(0, temp_dir_.absolutePath() + "/");
                    dependency_file_name.append(".dependency_");
                    qDebug() << "dependency_file_name: " << dependency_file_name;

                    QByteArray temp_folder_data = encryptor_.readFile(dependency_file_name);
                    QByteArray temp_folder_key  = encryptor_.decryptRSA(pvt_key, temp_folder_data);

                    new_decrypt_file_name       = data_folder_.DecryptFolder( temp_dir_.absolutePath() + "/" + new_files.at(i),\
                                                                                false,\
                                                                                temp_folder_key);

                    qDebug() << "new_decrypt_file_name: " << new_decrypt_file_name;

                    /* encrypt folder using repo key & move to repo folder */
                    EncryptFolder( new_decrypt_file_name, \
                                    (output_path_global + original_file_name),\
                                    repo_folder_key);

                    /* delete files */
                    data_folder_.deleteFile(temp_dir_.absolutePath() + "/" + new_files.at(i), !isDir, temp_del_dir_);
                    data_folder_.deleteFile(dependency_file_name,   !isDir, temp_del_dir_);
                    data_folder_.deleteFile(new_decrypt_file_name,  !isDir, temp_del_dir_);

                    /* log received file to db */
                    qDebug() << "Task::processData() - logged received file name: " << original_file_name;
                    storage_.writeFileNamesToDB( original_file_name );

                    qDebug() << "Completed";
                    emit completed();
                }
            }
        }
    }
    // free memory
    encryptor_.freeRSAKey(pvt_key);

    /* send all files to off-shore storage server */
    qDebug() << "Task::processData() - Shipping data to off-shore storage server :)";
    storage_.store(  file_path_ );
}

void Task::retrieveBiometricData()
{
    /*  assert Populated(requested_modalities_list_)
     *  traverse directory -  decrypts, decompresses and
     *  collects data to store modality corresponding 'requested' directories
     */
    if( !requested_modalities_list_.isEmpty() )
    {
        /*****************************************************************************
         *
         * (1) First download data from NAS share
         *     (2) Mount a single data file to PATH
         *         (3) Remove data and decompressed files from PATH
         *             (4) Repeat process until all files known have been extracted
         *                 (5) Announce that data is ready for retrieval
         *                      (6) Remove ready data file from host
         *
         ******************************************************************************/

        while( storage_.fetch() )
        {
            /* extract requested modality from single file on directory */
            for( int i = 0; i < requested_modalities_list_.size(); i++ )
            {
                qDebug() << "Task::retrieveBiometricData() - Looking for: " \
                         <<  requested_modalities_list_.at(i);
                data_folder_.traverseDirectory( requested_modalities_list_.at(i) , temp_del_dir_);
                qDebug() << "tRAVERSED DIR";

                /* Check for empty 'requested data directories and remove */
                /* Combine folders into one directory */
                data_folder_.packageAllRequestedModalities(requested_modalities_list_.at(i));
                qDebug() << "Packaged Modalities";
            }

            /* regex to remove received data file */
            QRegularExpression regex_main_data_file("\\d+\\-\\d+\\_\\d+\\-\\d+\\-\\d+.zip");
            /* regex to remove decompressed file of received data file */
            QRegularExpression regex_decompressed_data_file("\\d+\\-\\d+\\_\\d+\\-\\d+\\-\\d+.zip_repo_temp.zip_decompressed");

            QDirIterator *file_path_it = new QDirIterator( file_path_, \
                                                           ( QDir::Dirs | \
                                                             QDir::NoDot | \
                                                             QDir::NoDotDot | \
                                                             QDir::NoDotAndDotDot \
                                                           ));

            while( file_path_it->hasNext() )
            {
                QString directory_name = file_path_it->next();

                qDebug() << "Task::retrieveBiometricData() - removing " << directory_name \
                         << " " << directory_name.split(file_path_+ "/").at(1) ;

                QRegularExpressionMatch match_one = regex_decompressed_data_file.match( directory_name.split(file_path_+"/").at(1) );
                QRegularExpressionMatch match_two = regex_main_data_file.match( directory_name.split(file_path_+"/").at(1) );

                if( match_one.hasMatch() )
                    data_folder_.deleteFile(directory_name, true, temp_del_dir_ );

                if( match_two.hasMatch() )
                    data_folder_.deleteFile(directory_name, true, temp_del_dir_ );
            }

            /* free memory */
            delete file_path_it;
        }

        if( QFile( file_path_ + "/retrievedModalityData.zip").exists )
        {
            data_folder_.CompressDir(file_path_ + "/retrievedModalityData.zip", file_path_ + "/requested_mods_combined");

            RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
            QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
            QByteArray repo_folder_key  = "MDSBRMP";// encryptor_.decryptRSA(pvt_key, repo_folder_data);

            data_folder_.EncryptFolder( file_path_ +"/retrievedModalityData.zip", \
                                        file_path_ + "/retrievedModalityData.zip", \
                                        repo_folder_key, temp_del_dir_);

            emit requestedModalitiesReady( file_path_ + "/retrievedModalityData.zip" );

            /* remove files after files have been sent */
            data_folder_.deleteFile(file_path_ + "/requested_mods_combined", true, temp_del_dir_);
            data_folder_.deleteFile(file_path_ + "/retrievedModalityData.zip", false, temp_del_dir_);
        } 

        /* free memory */
        encryptor_.freeRSAKey(pvt_key);

    }
    else
    {
        qDebug() << "Task::retrieveBiometricData() - requested modality list empty";
    }
}

QString Task::getModalityName(QString modalityNumber)
{
    switch( modalityNumber.toInt())
    {
        case EAR2D          : return "Ear2D";
        break;
        case IRIS           : return "Iris";
        break;
        case EAR3D          : return "Ear3D";
        break;
        case FOOTPRINTS     : return "Footprints";
        break;
        case PALMPRINTS     : return "Palmprints";
        break;
        case FINGERPRINTS   : return "Fingerprints";
        break;
        case MICROSCOPE     : return "Microscope";
        break;
        default: // should never reach here
            qDebug() << "Task::getModalityName(): Fatal error - Unknown Modality...";
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
        if( !sourceDB->select( table, select_list, result, column_list, value_list) )
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
