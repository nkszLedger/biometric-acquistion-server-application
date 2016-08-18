#include <QDebug>
#include <server.h>
#include <QCoreApplication>

#include <task.h>
#include <encrypto.h>

/*!
 * \brief test_decrypt_repo_folder
 */
void test_decrypt_repo_folder()
{
    Encrypto encryptor_;
    RSA* pvt_key = encryptor_.getPrivateKey("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/dependency_.prvt");
    QByteArray repo_folder_data = encryptor_.readFile("/home/esaith/Documents/MinorsProject/BiometricAcquistionServerApp/DEPENDENCIES/Briefcase.dependency_");
    QByteArray repo_folder_key = encryptor_.decryptRSA(pvt_key, repo_folder_data);
    qDebug() << "repo_folder_key: " << repo_folder_key;

    Task myTask;
    QString repo_decrypt_file_name = myTask.DecryptFolder("/home/esaith/Downloads/DATA/102_ENCRYPTED.zip", \
                                                          true,\
                                                          repo_folder_key);

    qDebug() << "repo_decrypt_file_name: " << repo_decrypt_file_name;

    // free memory
    encryptor_.freeRSAKey(pvt_key);
}

#include <QDir>
void testDir(QString t_dir)
{
    QDir myDir;
    myDir.setPath(t_dir);
    if(myDir.exists())
        qDebug() << "YES";
    else
    {
        qDebug() << "NO";
        myDir.mkdir(t_dir);
        testDir(t_dir);
    }
}

/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //test_decrypt_repo_folder();

      testDir("/home/esaith/Downloads/1/11");


    //Server server;
    //server.StartServer();

    return a.exec();
}
