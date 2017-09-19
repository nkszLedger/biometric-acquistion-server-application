#include <QDebug>
#include <server.h>
#include <QCoreApplication>

#include <task.h>
#include <encrypto.h>

/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    server.StartServer();

    //QObject::connect(&a, SIGNAL(aboutToQuit()),&server, SLOT(serverOnClose()) );

    return a.exec();
}
