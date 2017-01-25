#include "sharedsettings.h"
#include <qdebug.h>
SharedSettings *SharedSettings::shared_settings_instance_ = 0;

SharedSettings::SharedSettings(QObject *parent) : QObject(parent)
{

}

QString SharedSettings::getCurrentLoggedInEmployeeID() const
{
    return current_loggedIn_employee_id_;
}

void SharedSettings::setCurrentLoggedInEmployeeID(const QString &currentLoggedInEmployeeID)
{
    current_loggedIn_employee_id_ = currentLoggedInEmployeeID;
}

QString SharedSettings::getRemoteDbDriver() const
{
    return remote_db_driver_;
}

void SharedSettings::setRemoteDbDriver(const QString &remote_db_driver)
{
    remote_db_driver_ = remote_db_driver;
}

QString SharedSettings::getRemoteDbName() const
{
    return remote_db_name_;
}

void SharedSettings::setRemoteDbName(const QString &remote_db_name)
{
    remote_db_name_ = remote_db_name;
}

QString SharedSettings::getRemoteDbPassword() const
{
    return remote_db_password_;
}

void SharedSettings::setRemoteDbPassword(const QString &remote_db_password)
{
    remote_db_password_ = remote_db_password;
}

QString SharedSettings::getRemoteDbUsername() const
{
    return remote_db_username_;
}

void SharedSettings::setRemoteDbUsername(const QString &remote_db_username)
{
    remote_db_username_ = remote_db_username;
}

int SharedSettings::getRemoteDbPort() const
{
    return remote_db_port_;
}

void SharedSettings::setRemoteDbPort(const int &remote_db_port)
{
    remote_db_port_ = remote_db_port;
}

QString SharedSettings::getRemoteDb() const
{
    return remote_db_;
}

void SharedSettings::setRemoteDb(const QString &remote_db)
{
    remote_db_ = remote_db;
}

QString SharedSettings::getLocalDbDriver() const
{
    return local_db_driver_;
}

void SharedSettings::setLocalDbDriver(const QString &LocalDbDriver)
{
    local_db_driver_ = LocalDbDriver;
}

QString SharedSettings::getLocalDbPath() const
{
    return local_db_path_;
}

void SharedSettings::setLocalDbPath(const QString &LocalDbPath)
{
    local_db_path_ = LocalDbPath;
}

QString SharedSettings::getPublicKeyPath() const
{
    return public_key_path_;
}

void SharedSettings::setPublicKeyPath(const QString &value)
{
    public_key_path_ = value;
}

QString SharedSettings::getServerPort() const
{
    return server_port_;
}

void SharedSettings::setServerPort(const QString &value)
{
    server_port_ = value;
}

QString SharedSettings::getServerIp() const
{
    return server_ip_;
}

void SharedSettings::setServerIp(const QString &value)
{
    server_ip_ = value;
}

QStringList SharedSettings::getOutputPathList() const
{
    return output_path_list_;
}

void SharedSettings::setOutputPathList(const QStringList &output_path_list)
{
    output_path_list_ = output_path_list;
}

QString SharedSettings::getCurrentParticipantID() const
{
    return current_participant_id_;
}

void SharedSettings::setCurrentParticipantID(const QString &current_participant_id)
{
    current_participant_id_ = current_participant_id;
}

void SharedSettings::setBiometricModalities(QStringList bioList)
{

    biometric_modalities_.clear();
    biometric_modalities_.append(bioList);
}

QStringList SharedSettings::getBiometricModalities()
{

    qDebug() << " - " << biometric_modalities_.size();
  for(int i =0; i < biometric_modalities_.size(); i++)
  {
    qDebug() << biometric_modalities_.at(i);
  }

  return biometric_modalities_;
}
