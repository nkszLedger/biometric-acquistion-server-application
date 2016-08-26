#include <QDir>
#include "task.h"
#include "shared.h"
#include <JlCompress.h>

Task::Task()
{
    // set the paths for temporary directories and repo directory
    temp_dir_.setPath(temp_path_global);
    repo_dir_.setPath(output_path_global);
}

void Task::run()
{
    //time consumer
    this->processData();
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
                }
            }


        }


    }
    // free memory
    encryptor_.freeRSAKey(pvt_key);
}
