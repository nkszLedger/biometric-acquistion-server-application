#ifndef SHAREDSETTINGS_H
#define SHAREDSETTINGS_H

#include <QObject>
#include <QStringList>

class SharedSettings : public QObject
{
  Q_OBJECT
public:
    /*!
    * \brief instance
    * \return
    */
    static SharedSettings *instance()
    {
      if (!shared_settings_instance_)
        shared_settings_instance_ = new SharedSettings;
      return shared_settings_instance_;
    }

    /*!
    * \brief setBiometricModalities
    * \param bioList
    */
    void setBiometricModalities(QStringList bioList);

    /*!
    * \brief getBiometricModalities
    * \return
    */
    QStringList getBiometricModalities();

    /*!
    * \brief getCurrentParticipantID
    * \return
    */
    QString getCurrentParticipantID() const;

    /*!
    * \brief setCurrentParticipantID
    * \param current_participant_id
    */
    void setCurrentParticipantID(const QString &current_participant_id);

    /*!
    * \brief getOutputPathList
    * \return
    */
    QStringList getOutputPathList() const;
    /*!
    * \brief setOutputPathList
    * \param output_path_list
    */
    void setOutputPathList(const QStringList &output_path_list);
    /*!
    * \brief getServerIp
    * \return
    */
    QString getServerIp() const;
    /*!
    * \brief setServerIp
    * \param value
    */
    void setServerIp(const QString &value);
    /*!
    * \brief getServerPort
    * \return
    */
    QString getServerPort() const;
    /*!
    * \brief setServerPort
    * \param value
    */
    void setServerPort(const QString &value);
    /*!
    * \brief getPublicKeyPath
    * \return
    */
    QString getPublicKeyPath() const;
    /*!
    * \brief setPublicKeyPath
    * \param value
    */
    void setPublicKeyPath(const QString &value);

    QString getLocalDbPath() const;
    void setLocalDbPath(const QString &localDbPath);

    QString getLocalDbDriver() const;
    void setLocalDbDriver(const QString &LocalDbDriver);

    QString getRemoteDb() const;
    void setRemoteDb(const QString &remote_db);

    int getRemoteDbPort() const;
    void setRemoteDbPort(const int &remote_db_port);

    QString getRemoteDbUsername() const;
    void setRemoteDbUsername(const QString &remote_db_username);

    QString getRemoteDbPassword() const;
    void setRemoteDbPassword(const QString &remote_db_password);

    QString getRemoteDbName() const;
    void setRemoteDbName(const QString &remote_db_name);

    QString getRemoteDbDriver() const;
    void setRemoteDbDriver(const QString &remote_db_driver);

    QString getCurrentLoggedInEmployeeID() const;
    void setCurrentLoggedInEmployeeID(const QString &currentLoggedInEmployeeID);

signals:

public slots:

private:
    QStringList biometric_modalities_;

    explicit SharedSettings(QObject *parent = 0);

    static SharedSettings *shared_settings_instance_;

    QString current_loggedIn_employee_id_;
    QString current_participant_id_;

    QStringList output_path_list_;

    QString server_ip_;
    QString server_port_;
    QString public_key_path_;

    QString local_db_path_;
    QString local_db_driver_;

    int remote_db_port_;
    QString remote_db_;
    QString remote_db_driver_;
    QString remote_db_username_;
    QString remote_db_password_;
    QString remote_db_name_;
};

#endif // SHAREDSETTINGS_H
