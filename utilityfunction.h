#ifndef UTILITYFUNCTION_H
#define UTILITYFUNCTION_H

#include <QFile>
#include <QDebug>
#include <QObject>
#include <encrypto.h>
#include <QStringList>
#include <QCoreApplication>

#include <sgfplib.h>

#include <dpfj.h>
#include <dpfpdd.h>
#include <dpfj_quality.h>
#include <dpfj_compression.h>

enum MODALITY_STATUS{
    CAPTURE_COMPLETED = 0,
    CAPTURE_INCOMPLETE = 1
};

class UtilityFunction : public QObject
{
    Q_OBJECT
public:
    /*!
    * \brief instance
    * \return
    */
    static UtilityFunction *instance()
    {
        if (!utility_functions_)
          utility_functions_ = new UtilityFunction;
        return utility_functions_;
    }

    /*!
    * \brief config_inputString (part of Load configuration function)
    * \param inLine
    * \return
    */
    bool config_inputString(const QString inLine);

    /*!
    * \brief fixPathString
    * \param filePath
    */
    void fixPathString(QString& filePath);

    /*!
    * \brief loadPaths
    * \param inLine
    * \param in
    * \param pathLists
    */
    void loadPaths(QString& inLine,\
                   QTextStream &in,\
                   QStringList &pathLists);

    /*!
    * \brief readFile
    * \param fileName
    * \return
    */
    QByteArray readFile(QString fileName);

    /*!
    * \brief writeFile
    * \param fileName
    * \param data
    */
    void writeFile(QString fileName, QByteArray &data);

    /*!
    * \brief unencryptPassword
    * \param encrypted
    * \return
    */
    QString unencryptPassword(QByteArray encrypted);

    /*!
    * \brief load Configuration file (Controller)
    * \param fileName
    * \param inputDir
    * \param outputDir
    * \return
    */
    int loadConfigFile(QStringList &inputDir, \
                       QStringList &outputDir, \
                       QString &local_database_path, \
                       QString &remote_database_path, \
                       QString &dbName, \
                       QString &user, \
                       QString &passwd, \
                       QString &port, \
                       QStringList &bioList, \
                       QString &serverIp, QString &serverPort, QString &publicKeyDest, \
                       QString fileName);
    /*!
    * \brief getSecugenQualityScore
    * \param Image
    * \param Width
    * \param Height
    * \param QualityScore
    * \return
    */
    int getSecugenQualityScore(const unsigned char *Image,
                               const int Width,
                               const int Height,
                               int &QualityScore);

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
    int save_to_wsq(const unsigned char *Image,
                    const unsigned int Width,
                    const unsigned int Height,
                    const unsigned int DPI,
                    const unsigned int BPP,
                    const QString FileName);

    /*!
    * \brief deletePath - deletes folder of single file
    * \param path
    */
    void deletePath(QString path);

    /*!
    * \brief serialiseSaveDirectory
    * \param biometricType
    */
    void serialiseSaveDirectory(QString biometricType);

    /*!
    * \brief CompressEncrypt
    * \param biometricType
    * \param inputPath
    * \param deleteOriginalFolder
    * \param tempEncryption
    */
    void CompressEncrypt(QString fullFileNameZip, \
                         QString inputPath,
                         bool deleteOriginalFolder, \
                         bool tempEncryption);
signals:

    /*!
    * \brief exitBiometricModalityCaptureForm
    */
    void exitBiometricModalityCaptureForm(MODALITY_STATUS);

    /*!
    * \brief continueToSaveFingerprints
    * \param path
    */
    void continueToSaveFingerprints(QString path);

    /*!
    * \brief continueToSaveIris
    * \param path
    */
    void continueToSaveIris(QString path);

    /*!
    * \brief continueToSavePalmprints
    * \param path
    */
    void continueToSavePalmprints(QString path);

    /*!
    * \brief continueToSaveEar3D
    * \param parentFolder
    * \param pathToPopulate
    */
    void continueToSaveEar3D(QString parentFolder,\
                             QString pathToPopulate);

private:
    /*!
    * \brief UtilityFunction - private constructor
    * \param parent
    */
    explicit UtilityFunction(QObject *parent = 0);

    /*!
    * \brief utility_functions_ - single instance of object
    */
    static UtilityFunction *utility_functions_;

};

#endif // UTILITYFUNCTION_H
