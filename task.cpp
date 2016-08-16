#include <QDir>
#include "task.h"
#include "shared.h"
#include <JlCompress.h>

Task::Task()
{
    temp_dir_.setPath(temp_path_global);
    repo_dir_.setPath(output_path_global);
}

void Task::run()
{
    //time consumer

    this->processData();

    //emit Result(socket_number_);
}

void Task::setSocketInput(int socketNumber)
{
    socket_number_ = socketNumber;
}

void Task::mergeFolders(QString inputFolderPath,\
                        QString existingFolderPath)
{
    // check if file exist of repository
    QDir input_folders_dir(inputFolderPath);

    QDir input_files_dir;
    QStringList input_files_list;

    QStringList input_folder_list = input_folders_dir.entryList();

    QString temp_str = "";
    QString temp_file_name = "";
    QFileInfo temp_file_info;
    QFileInfo repo_folder;

    for(int i = 0; i < input_folder_list.size(); i++)
    {
        temp_file_name =  input_folder_list.at(i);

        if(temp_file_name == "." || temp_file_name == ".." )
        {
            continue;
        }
        else
        {
            // set file name (new files)
            temp_file_info.setFile(inputFolderPath + "/" + temp_file_name);

            // check if directory
            if(temp_file_info.isDir())
            {
                // set folder name repo
                repo_folder.setFile(existingFolderPath+"/"+temp_file_name);

                // check if directory with same heading exists in repo folder
                if(repo_folder.exists())
                {
                    // loop through all file in input file and move to repo folder
                    input_files_list.clear();
                    // get listings of files
                    input_files_dir.setPath(inputFolderPath+"/"+temp_file_name);
                    input_files_list =input_files_dir.entryList();

                    for(int j = 0; j < input_files_list.size(); j++)
                    {

                        if(input_files_list.at(j) == "." || input_files_list.at(j)  == ".." )
                        {
                            continue;
                        }
                        else
                        {
                            // move file to repo folder
                            input_files_dir.rename((inputFolderPath + "/" + temp_file_name + "/" + input_files_list.at(j)),
                                                   (existingFolderPath+"/"+temp_file_name +"/"+ input_files_list.at(j)));
                        }

                    }
                }
                else
                {
                    // move entire folder to repo folder
                    input_folders_dir.rename((inputFolderPath + "/" + temp_file_name),
                                             (existingFolderPath+"/"+ temp_file_name));
                }

            }
        }

    }
}

void Task::CompressDir(QString zipFile, QString directory)
{

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
    QStringList list = JlCompress::extractDir(zipFile, directory);
}

QString Task::DecryptFolder(QString fileName, bool repoFile)
{
    // read in encrypted file
    QByteArray encrypted_file = encryptor_.readFile(fileName);

    //load passphrase
    //    passphrase_ = decryptPassphrase();
    QByteArray decrypted_file = encryptor_.decryptAES(passphrase_.toLatin1(), encrypted_file);

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
                         QString outputFileName)
{
    // read in encrypted file
    QByteArray file = encryptor_.readFile(inputFileName);

    //load passphrase
    //    passphrase_ = decryptPassphrase();
    QByteArray encrypted_file = encryptor_.encryptAES(passphrase_.toLatin1(), file, false);

    //write decrypted file
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
    // check if file exist of repository
    QStringList new_files  = temp_dir_.entryList();
    QStringList repo_files = repo_dir_.entryList();

    QString temp_merged_file_name  = "";

    QString new_decrypt_file_name  = "";
    QString repo_decrypt_file_name = "";

    QString new_decompressed_file_name  = "";
    QString repo_decompressed_file_name = "";

    QString temp_file_name     = "";
    QString original_file_name = "";

    QFile repo_file;

    for(int i = 0; i < new_files.size(); i++)
    {
        temp_file_name =  new_files.at(i);

        if(temp_file_name == "." || temp_file_name == ".." )
        {
            continue;
        }
        else
        {
            original_file_name = new_files.at(i).split("#").at(1);

            repo_file.setFileName(output_path_global + original_file_name);

            if(repo_file.exists())
            {

                // if exists
                // decrypt folder on repo
                repo_decrypt_file_name = DecryptFolder(output_path_global + original_file_name, true);


                //unzip existing file
                repo_decompressed_file_name = repo_decrypt_file_name+"_decompressed";
                DecompressDir(repo_decrypt_file_name, repo_decompressed_file_name);

                // decrypt new folder
                new_decrypt_file_name = DecryptFolder(temp_dir_.absolutePath() + "/" + new_files.at(i), false);


                //unzip new file
                new_decompressed_file_name = new_decrypt_file_name+"_decompressed";
                DecompressDir(new_decrypt_file_name, new_decompressed_file_name);


                // merge folders
                mergeFolders(new_decompressed_file_name, repo_decompressed_file_name);


                // compress folder
                temp_merged_file_name = repo_decompressed_file_name + "_compressed.zip";
                CompressDir(temp_merged_file_name, repo_decompressed_file_name);


                // remove temporay files
                deleteFile(repo_decrypt_file_name, false);
                deleteFile(repo_decompressed_file_name, true);
                deleteFile(new_decrypt_file_name, false);
                deleteFile(new_decompressed_file_name, true);
                deleteFile(output_path_global + original_file_name, false);
                deleteFile(temp_dir_.absolutePath() + "/" + new_files.at(i), false);

                // encrypt
                EncryptFolder(temp_merged_file_name, (output_path_global + original_file_name));

                // delete zip file only after encryption [Others must be deleted before]
                deleteFile(temp_merged_file_name, false);

                // send signal of completion
                qDebug() << "About to emit";
                emit completed();

            }
            else
            {
                // move to repo folder
                temp_dir_.rename(temp_file_name, (output_path_global + original_file_name));
            }
        }


    }
}

void Task::setPassPhrase(QString passphrase)
{
    passphrase_ = passphrase;
}
