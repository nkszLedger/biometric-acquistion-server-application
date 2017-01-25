#include "database.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <encrypto.h>


/*!
 * \brief - Database constructor
 *
 * \param - QObject *parent (Input)
 *
 */
Database::Database( QObject *parent ) : QObject( parent )
{

}

Database::Database(QString driver, QObject *parent): QObject( parent )
{
    setDriver(driver);
}


/*!
 * \brief 	- Establishes a connection to database
 *
 * \return bool - true if successfully connects to database
 *              - false otherwise
 */
bool Database::connOpen()
{
    return initPostGres();

}

/*!
 * \brief  - Closes a connection to database
 *
 * \return void
 */
void Database::connClosed()
{
    database.close();
    database.removeDatabase(QSqlDatabase::defaultConnection);
}

/*!
 * \brief    	  - Converts vector into merged string tokens
 *                   seperated by the comma delimeter
 *
 * \param    	  - QVector<QString> value (Input)
 *
 * \return bool   - QString strBuilder
 */
QString Database::toString( QVector<QString> &value )
{
   QString strBuilder = "";

   for(int i = 0 ; i<value.size() ; i++)
   {
       if( value.size() == 1)
       {
           strBuilder += value[i];
           return strBuilder;
       }

       else if ( value.size() > 1 && i < (value.size() - 1) )
       {
           strBuilder += value[i];
           strBuilder += ", ";
       }
       else
       {
           strBuilder += value[i];
           return strBuilder;
       }
   }
}


/*!
 * \brief	- Initializes the Postgresql database
 *	           with all required connection strings
 *
 * \return bool - true if successfully initializes database object
 * 	        - false otherwise
 */
bool Database::initPostGres()
{
    database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName("localhost");
    database.setPort(5432);
    database.setDatabaseName("biometricapplication");
    database.setPassword("BioAcqApp2016");
    database.setUserName("postgres");


    if(!database.open())
    {
        qDebug() << "Database::initPostGres() - Failed to open Postgres DB";
        return false;
    }
    else
    {
        qDebug() << "Database: Postgres Connected..... ";
        return true;
    }
}

/*!
 * \brief 		- Database Object accessor
 *
 * \return QSqlDatabase -  database object
 *
 */
QSqlDatabase Database::getDatabase() const
{
    return database;
}

/*!
 * \brief - Database Object mutator
 *
 * \param - QSqlDatabase object (Input)
 *
 * \return void
 *
 */
void Database::setDatabase( const QSqlDatabase &value )
{
    database = value;
}

/*!
 * \brief 		- Database host name accessor
 *
 * \return QString 	- Host name
 *
 */
QString Database::getHost() const
{
    return host;
}

/*!
 * \brief - Database host name mutator
 *
 * \param - QString host name (Input)
 *
 * \return void
 *
 */
void Database::setHost( const QString &value )
{
    host = value;
}

/*!
 * \brief 	     - Database driver name accessor
 *
 * \return QString   - Driver name
 *
 */
QString Database::getDriver() const
{
    return driver;
}

/*!
 * \brief - Database driver name mutator
 *
 * \param - QString driver name (Input)
 *
 * \return void
 *
 */
void Database::setDriver( const QString &value )
{
    driver = value;
}

/*!
 * \brief	- Checks if database driver is Postgresql
 *
 * \return bool	- true if driver is Postgresql
 *         	- false if otherwise
 */
bool Database::isPostGres()
{
    if(getDriver() == "QPSQL")
        return true;
    else
        return false;
}

/*!
 * \brief 	    - Database name accesor
 *
 * \return QString  - Database name
 *
 */
QString Database::getDbname() const
{
    return dbname;
}

/*!
 * \brief - Database name mutator
 *
 * \param - QString database name (Input)
 *
 * \return void
 *
 */
void Database::setDbname( const QString &value )
{
    dbname = value;
}

/*!
 * \brief 	    - Database username accesor
 *
 * \return QString  - Database name
 *
 */
QString Database::getUsername() const
{
    return username;
}

/*!
 * \brief - Database username mutator
 *
 * \param - QString username (Input)
 *
 * \return void
 *
 */
void Database::setUsername( const QString &value )
{
    username = value;
}

/*!
 * \brief 	    - Database password accesor
 *
 * \return QString  - Password
 *
 */
QString Database::getPassword() const
{
    return password;
}

/*!
 * \brief - Database password mutator
 *
 * \param - QString password (Input)
 *
 * \return void
 *
 */
void Database::setPassword( const QString &value )
{
    password = value;
}

/*!
 * \brief 	- Database port number accesor
 *
 * \return int  - Port number
 *
 */
int Database::getPort() const
{
    return port;
}

/*!
 * \brief - Database port number mutator
 *
 * \param - int port number (Input)
 *
 * \return void
 *
 */
void Database::setPort( int value )
{
    port = value;
}



/*!
 * \brief Select function                    - Retrieves data from database table
 *
 * \param QString table                      - specifies name of the db table (Input)
 * \param QVector<QString> select_columns    - specifies fields to fetch data from (Input)
 * \param QVector<QString> column_list       - specifies which columns to filter by (Input)
 * \param QVector<QString> value_list        - specifies the values for the columns (Input)
 * \param QVector<QSqlRecord> result         - a vector to store results (Output)
 *
 * \return bool           		     - true if query executed successfully
 *                          		     - false if query execution failed
 */
bool Database::select(  QString table,
                        QVector<QString> &select_columns,
                        QVector<QString> &column_list,
                        QVector<QString> &value_list,
                        QVector<QSqlRecord> &result
                      )
{
    // clear the results
    result.clear();

    // link database to query
    QSqlQuery query(getDatabase());

    // check if query is executed
    bool isQueryExecuted;

    // declare a record
    QSqlRecord rec;

    QString strSelectList = toString(select_columns);
    //QString strColumnList = toString(column_list) ;

    QString strQuery = "";

    if( !(column_list.isEmpty() && value_list.isEmpty()) )
    {
        strQuery = "SELECT " + strSelectList + " FROM " + table +
                   " WHERE ";

        // string query builder
        for( int i = 0 ; i < column_list.size() ; i++)
        {

            if ( column_list.size() == 1 || i == (value_list.size() - 1) )
            {
                strQuery += ( column_list[i] + "=:" + column_list[i] );

            }
            else
                strQuery += (column_list[i] + "=:" + column_list[i] + " AND ");
        }


        // prepare query
        query.prepare(strQuery);

        // bind parameters
        for (int i = 0; i < column_list.size() ; i++)
        {
            qDebug() << " " << value_list[i].toStdString().c_str();
            query.bindValue(":"+column_list[i], value_list[i].toStdString().c_str());
        }

    }
    else
    {
        strQuery = " SELECT " + strSelectList + " FROM " + table;

        // prepare query
        query.prepare(strQuery);

    }

    qDebug() << "Select Query : " << strQuery;
    // execute query
    isQueryExecuted = query.exec();

    if( !isQueryExecuted )
    {
        qDebug() << "database::select() query.exec() Error: " << query.lastError().text();
        qDebug() << "database error code: " << query.lastError().number();
        return isQueryExecuted;
    }

    // save records to the results vector
    while(query.next())
    {
        result.push_back(query.record());
    }


    return isQueryExecuted;
}
