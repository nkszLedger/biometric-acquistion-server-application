#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QVector>
#include <QObject>
#include <QSqlQuery>
#include <QSqlDatabase>


/*!
 * \brief The Database class
 */
class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = 0);

    Database(QString driver, QObject *parent = 0);

    /*!
    * \brief researcherLogin
    * \param userName
    * \param password
    * \return
    */
    int researcherLogin(QString userName,
                        QString password);
    /*!
    * \brief connClosed
    */
    void connClosed();
    /*!
    * \brief setPort
    * \param value
    */
    void setPort(int value);
    /*!
    * \brief setHost
    * \param value
    */
    void setHost(const QString &value);
    /*!
    * \brief setDbname
    * \param value
    */
    void setDbname(const QString &value);
    /*!
    * \brief setDriver
    * \param value
    */
    void setDriver(const QString &value);
    /*!
    * \brief setUsername
    * \param value
    */
    void setUsername(const QString &value);
    /*!
    * \brief setPassword
    * \param value
    */
    void setPassword(const QString &value);
    /*!
    * \brief setDatabase
    * \param value
    */
    void setDatabase(const QSqlDatabase &value);
    /*!
    * \brief connOpen
    * \return
    */
    bool connOpen();
    /*!
    * \brief testSQLite
    * \return
    */
    bool initSQLite();
    /*!
    * \brief isPostGres
    * \return
    */
    bool isPostGres();
    /*!
    * \brief testPostGres
    * \return
    */
    bool initPostGres();

    // *************** Helper Functions *****************
    /*!
    * \brief select
    * \param table
    * \param select_columns
    * \param column_list
    * \param value_list
    * \param result
    * \return
    */
    bool select(QString table,
                QVector<QString> &select_columns,
                QVector<QString> &column_list,
                QVector<QString> &value_list,
                QVector<QSqlRecord> &result);


    // *************** End Of Helper Functions *****************


    /*!
    * \brief getPort
    * \return
    */
    int getPort() const;
    /*!
    * \brief getHost
    * \return
    */
    QString getHost() const;
    /*!
    * \brief getDriver
    * \return
    */
    QString getDriver() const;
    /*!
    * \brief getDbname
    * \return
    */
    QString getDbname() const;
    /*!
    * \brief getUsername
    * \return
    */
    QString getUsername() const;
    /*!
    * \brief getPassword
    * \return
    */
    QString getPassword() const;
    /*!
    * \brief getDatabase
    * \return
    */
    QSqlDatabase getDatabase() const;

    bool selectWithCondition(QString table,
                             QVector<QString> &select_columns,
                             QVector<QString> &column_list,
                             QVector<QString> &value_list,
                             QString columnCondition,
                             QString conditionValue,
                             QVector<QSqlRecord> &result);
signals:

public slots:

private:

    int port;
    QString driver;
    QString host;
    QString dbname;
    QString username;
    QString password;
    QSqlDatabase database;

    /*!
     * \brief toString
     * \param value
     * \return
     */
    QString toString(QVector<QString> &value);
};

 #endif // DATABASE_H
