#include "UtilityFunction.h"

#include <QDir>
#include <Zipper/Include/>
#include <QFileInfo>
#include <encrypto.h>
#include <sharedsettings.h>

// set single instance
UtilityFunction *UtilityFunction::utility_functions_ = 0;

bool UtilityFunction::config_inputString(const QString inLine)
{
    // check if comment
    if(inLine.startsWith("#") ||\
            (inLine.isEmpty()))
        return false;
    else
        return true;
}

void UtilityFunction::fixPathString(QString &filePath)
{
    // -> replace \\ with /
    while(filePath.contains("\\"))
        filePath.replace("\\", "/");
}

void UtilityFunction::loadPaths(QString &inLine, QTextStream &in, QStringList &pathLists)
{
    QString in_line = inLine;

       int i     = 0;
       int limit = QString(in_line.split(":").at(1)).toInt();

       // loop through input directories
       while(i < limit)
       {
           // - get line
           in_line = in.readLine();
           if(config_inputString(in_line))
           {
               // -- get current location of executable
               if(in_line.mid(0, 8) == "resources")
                   in_line.insert(0, QString("%1\\").arg(QCoreApplication::applicationDirPath()));

               // -- fix string
               fixPathString(in_line);

               // -- append to list
               pathLists.append(in_line);
               i++;
           }
       }
}

void UtilityFunction::writeFile(QString fileName, QByteArray &data)
{
  QFile file(fileName);
  if(!file.open(QFile::WriteOnly))
  {
      Logger::instance()->error("UtilityFunction::writeFile() - Could not write file");
      qDebug() << "Could not write file";
      return;
  }

  file.write(data);
  file.close();
}


QByteArray UtilityFunction::readFile(QString fileName)
{
    // fix string
     fixPathString(fileName);

     QByteArray data;
     QFile file(fileName);
     if(!file.open(QFile::ReadOnly))
     {
         Logger::instance()->error("UtilityFunction::readFile() - Could not read file");
         qDebug() << "Could not read file";
         return data;
     }

     // read all data into byte array
     data = file.readAll();

     file.close();
     return data;
}

QString UtilityFunction::unencryptPassword(QByteArray encrypted)
{
    Encrypto Encrypt_module;

    // get decrypted string
    QByteArray decrypted = Encrypt_module.decryptAES(QString("MDSBRMP").toLatin1(), encrypted);

    return (QString)decrypted;
}

// -------------------
// SecugenQualityScore
// -------------------
int UtilityFunction::getSecugenQualityScore(const unsigned char* Image,\
                                            const int            Width,\
                                            const int            Height,\
                                            int&                 QualityScore)
{
  // Secugen Object
  HSGFPM secugen_object;

  // integer variable
  DWORD rtrn_vl       = 0;
  DWORD quality_score = 0;
  // integer constants
  const int kZero       = 0;
  const int kMinusOne   = -1;
  const int kMinusThree = -3;

  // create instance of Secugen object
  rtrn_vl = SGFPM_Create(&secugen_object);

  // check if previous function was successful
  if((int)rtrn_vl != 0)
    return kMinusThree;

  // calculate quality
  rtrn_vl = SGFPM_GetImageQuality(secugen_object,\
                                  (DWORD)Width,\
                                  (DWORD)Height,\
                                  (BYTE*)Image,\
                                  &quality_score);

  QualityScore = static_cast<int>(quality_score);

  // check if previous function was successful
  if((int)rtrn_vl != 0)
    return kMinusOne;

  // terminate secugn object
  rtrn_vl = SGFPM_Terminate(secugen_object);

  // check if previous function was successful
  if((int)rtrn_vl != 0)
    return kMinusThree;

  return kZero;
}

/*!
 * \brief save_to_wsq
 * \param Image
 * \param Width
 * \param Height
 * \param DPI
 * \param BPP
 * \param FileName
 * \return
 */
int UtilityFunction::save_to_wsq(   const unsigned char* Image, \
                                    const unsigned int   Width, \
                                    const unsigned int   Height, \
                                    const unsigned int   DPI, \
                                    const unsigned int   BPP, \
                                    const QString        FileName)
{
    // int
    // - variable
    int error_code = 0;
    // integer constants
    const int kZero       = 0;
    const int kMinusOne   = -1;
    const int kMinusTwo   = -2;
    const int kMinusFour  = -4;
    const int kMinusThree = -3;
    // unsigned integer
    unsigned int bitrate      = 419;
    unsigned int tolerance    = 1;
    // unsigned variables
    unsigned char* image_data = new unsigned char[(Width * Height)];
    unsigned int  image_size = (Width * Height);

    // start compression
    error_code = dpfj_start_compression();

    if(error_code != DPFJ_SUCCESS)
        return kMinusOne;

    // set wsq bit rate
    error_code = dpfj_set_wsq_bitrate(bitrate, tolerance);

    if(error_code != DPFJ_SUCCESS)
        return kMinusTwo;

    // convert to wsq
    error_code = dpfj_compress_raw(Image,(Width * Height), Width, Height, DPI, BPP, DPFJ_COMPRESSION_WSQ_NIST);

    if(error_code != DPFJ_SUCCESS)
        return kMinusThree;

    // get processed data
    error_code = dpfj_get_processed_data(image_data,&image_size);

    if(error_code != DPFJ_SUCCESS)
        return kMinusThree;

    QFile file(FileName);

    if(file.open(QIODevice::WriteOnly))
        file.write((char*)image_data,(qint64)image_size);
    else
        return kMinusFour;

    // release memory
    dpfj_finish_compression();

    return kZero;
}

/*!
 * \brief deletePath - deletes a directory or specific file
 * \param path
 */
void UtilityFunction::deletePath(QString path)
{
  // set dir
  QDir temp_dir(path);

  // set file info
  QFileInfo temp_file;
  temp_file.setFile(path);

  //check
  if(temp_file.isDir())
  {
    temp_dir.removeRecursively();
  }
  else
  {
    temp_dir.remove(path);
  }
}

void UtilityFunction::serialiseSaveDirectory(QString biometricType)
{
    int result = 0;
    bool temp_encryption = true;
    bool delete_original_folder = true;

    // get path
    QString path = SharedSettings::instance()->getOutputPathList().at(0);
    // get file
    QString file = SharedSettings::instance()->getCurrentParticipantID();

    // decrypt file
    Encrypto::fullDecryption(path+"/"+file+".zip",\
                             path+"/"+file+".zip",\
                             path+"/"+file+".dependency_",\
                             temp_encryption,\
                             "");

    // decompress file
    result = Zipper::decompressDir(path+"/"+file+".zip",\
                                       path+"/"+file, \
                                       delete_original_folder);

    if(result == 0)
    {
        // QDir
        QDir current_dir;
        current_dir.setPath(path+"/"+file+"/"+biometricType);

        // create dir
        if(current_dir.exists() || \
                current_dir.mkdir(path+"/"+file+"/"+biometricType))
        {
            //emit differrent signal per modality
            if(biometricType == "Fingerprints")
                emit continueToSaveFingerprints(path+"/"+file+"/"+biometricType);
            else if(biometricType == "Iris")
                emit continueToSaveIris(path+"/"+file+"/"+biometricType);
            else if(biometricType == "Palmprints")
                emit continueToSavePalmprints(path+"/"+file+"/"+biometricType);
            else if(biometricType == "Ear3D")
                emit continueToSaveEar3D(path+"/"+file,\
                                         path+"/"+file+"/"+biometricType);
        }

        if(biometricType != "Ear3D" )
        {
            CompressEncrypt(path+"/"+file+".zip",\
                            path+"/"+file, \
                            delete_original_folder, \
                            temp_encryption);
        }
    }
}

void UtilityFunction::CompressEncrypt(QString fullFileNameZip, \
                                      QString inputPath,
                                      bool deleteOriginalFolder, \
                                      bool tempEncryption)
{
    // zip folder
    Zipper::compressDir(fullFileNameZip,\
                        inputPath,\
                        deleteOriginalFolder);

    // encrypt folder
    Encrypto::fullEncryption(fullFileNameZip,\
                             tempEncryption,\
                             "");
}

int UtilityFunction::loadConfigFile(QStringList &inputDir, \
                                    QStringList &outputDir, \
                                    QString &local_database_path, \
                                    QString &remote_database_path, \
                                    QString &dbName, \
                                    QString &user, \
                                    QString &passwd, \
                                    QString &port, \
                                    QStringList &bioList, \
                                    QString &serverIp,\
                                    QString &serverPort,\
                                    QString &publicKeyDest,\
                                    QString fileName)
{
    // QString
    QString temp_string_2;
    QString in_line     = "";
    QStringList temp_string;

    // const
    int kZero  = 0;
    int kOne   = 1;
    int kTwo   = 2;
    int kThree = 3;
    int kFour  = 4;
    // int
    int i     = 0;
    int limit = 0;

    // create instance of file
    QFile config_file(fileName);

    // check if file exists
    if(config_file.exists())
    {
        // - load file
        if(config_file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&config_file);

            // -- load string
            in_line = in.readLine();

            // -- check file declaration
            if(in_line != "<config>")
                return -3;

            // -- go through entire file
            while((!in.atEnd()))
            {
                // --- load string
                in_line = in.readLine();

                // --- check if end of file
                if(in_line == "<config/>")
                    break;

                // --- Skip comments
                if(config_inputString(in_line))
                {
                    // ---- check
                    if(in_line.contains("Input"))
                    {
                        loadPaths(in_line, in, inputDir);
                    }
                    else if(in_line.contains("Output"))
                    {
                        loadPaths(in_line, in, outputDir);
                    }
                    else if(in_line.contains("SQLITE"))
                    {
                        // ----- get SQL-Lite
                        in_line = in.readLine();
                        if(config_inputString(in_line))
                        {
                            // ------- get current location of executable
//                            if(in_line.mid(0, 8) == "resources")
                                in_line.insert(0, QString("%1/").arg(QCoreApplication::applicationDirPath()));

                            fixPathString(in_line);

                            local_database_path = in_line;
                        }
                    }
                    else if(in_line.contains("POSTGRES"))
                    {
                        // ----- get SPostgres
                        temp_string = in.readLine().split(",");

                        if(config_inputString(in_line))
                        {
                            port                 = temp_string.at(kOne);
                            user                 = temp_string.at(kThree);
                            dbName               = temp_string.at(kTwo);
                            QString path_to_password = QString("%1/").arg(QCoreApplication::applicationDirPath());
                            path_to_password.append(temp_string.at(kFour));
                            qDebug() << "path_to_password: " << path_to_password;
                            passwd               = unencryptPassword(readFile(path_to_password));
                            remote_database_path = temp_string.at(kZero);
                        }
                    }
                    else if(QString::compare(in_line.mid(0, 11), ("Biometrics:")) == 0)
                    {
                        // ----- get biometric modalities
                        temp_string = in_line.split(":");
                        temp_string_2 = temp_string.at(1);
                        bioList = temp_string_2.split(",");
                    }
                    else if(in_line.contains("Server"))
                    {
                        // ----- get Server
                        temp_string = in.readLine().split(",");

                        serverIp   = temp_string.at(kZero);
                        serverPort = temp_string.at(kOne);
                    }
                    else if(in_line.contains("PEM"))
                    {
                        // ----- get PEM
                        publicKeyDest = in.readLine();
                        publicKeyDest.insert(0, QString("%1/").arg(QCoreApplication::applicationDirPath()));
                    }
                }
            }
        }
        else
        {
            // -- return error - cannot open file
            signed minusTwo = -2;
            return minusTwo;
        }

    }
    else
    {
        // - return error - file not found
        signed minusOne = -1;
        return minusOne;
    }

    return 0;
}

UtilityFunction::UtilityFunction(QObject *parent) : QObject(parent)
{

}
