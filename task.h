#ifndef TASK_H
#define TASK_H

#include "folder.h"
#include "storage.h"

#include <QDir>
#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <QStringList>

class Task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Task();

    /*!
     * \brief setSocketInput
     * \param socketNumber
     */
    void setSocketInput(int socketNumber);

    /*!
     * \brief auto_retrieve
     * \return
     */
    bool autoRetrieve() const;

    /*!
     * \brief setAutoRetrieve
     * \param requested_modalities_list
     * \param autoRetrieve
     */
    void setAutoRetrieve(QStringList &requestedModalitiesList, \
                          bool autoRetrieve);
signals:
    /*!
     * \brief completed
     */
    void completed();
    /*!
     * \brief requestedModalitiesReady
     * \param requestedModalitiesfilePath
     */
    void requestedModalitiesReady(QString requestedModalitiesfilePath);

public slots:

protected:
    void run();

private:
    QDir repo_dir_;
    QDir temp_dir_;
    QDir temp_del_dir_;

    int socket_number_;
    bool auto_retrieve_;

    QString file_path_;
    QStringList requested_modalities_list_;
    QString requested_modalities_dir_path_;

    Folder data_folder_;
    Encrypto encryptor_;
    Storage storage_;

    /*!
     * \brief getModalityName
     * \param modality
     * \return
     */
    QString getModalityName(QString modalityNumber);

    /*!
     * \brief processData
     */
    void processData();

    /*!
     * \brief retrieveBiometricData
     */
    void retrieveBiometricData();
};

#endif // TASK_H
