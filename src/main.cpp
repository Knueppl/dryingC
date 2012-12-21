#include "IOHandler.h"
#include "Client.h"
#include "DryingControl.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

const char* LOCK_FILE = "/etc/dryingC/daemon.lock";
const char* PID_FILE  = "/var/run/dryingC.pid";


/* signal handler, says qapplication have to clean up and quit. */
void signal_handler(int)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (!QCoreApplication::instance())
        exit(1);

    QCoreApplication::exit();
}

int main(int argc, char** argv)
{
    /* If lock file does exist, run as client. */
    if (QFile::exists(PID_FILE))
    {
        qDebug() << "Pid file \"" << PID_FILE << "\" exists. Run as client.";

        QCoreApplication app(argc, argv);
        Client client;

        return app.exec();
    }

    /* Fork and exit to a be a daemon. You know ... */
    pid_t pid = fork();

    if (pid < 0)
    {
        qDebug() << *argv << ": fork fails.";
        return 1;
    }
    if (pid > 0)
    {
        qDebug() << *argv << ": daemon started.";
        return 0;
    }


    /* Capture SIGINT and SIGTERM to clean up before was killed. Maintainly to remove deamon.lock file. */
    struct sigaction action;

    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    /* Create daemon.lock file. So a second app will run as client. */
    umask(0);
    QFile file(LOCK_FILE);

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << *argv << ": can't create lock file \"" << LOCK_FILE << "\".";
    }

    file.close();

    /* get new pid and create a pid file for systemd. */
    pid_t sid = setsid();
    file.setFileName("/var/run/dryingC.pid");

    if (!file.open(QIODevice::WriteOnly))
    {
        QFile::remove(LOCK_FILE);
        qDebug() << *argv << ": can't create \"" << PID_FILE << "\".";
        return 1;
    }

    file.write(QByteArray::number(sid).data());
    file.close();


    /* start daemon process */
    QCoreApplication app(argc, argv);
    DryingControl control("/etc/dryingC/config.xml");

    app.exec();

    /* clean up */
    QFile::remove(PID_FILE);

    if (!QFile::remove(LOCK_FILE))
    {
        qDebug() << *argv << ": can't remove lock file \"" << LOCK_FILE << "\".";
        return 1;
    }

    return 0;
}

