#include "IOHandler.h"
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


/* signal handler */
void signal_handler(int)
{
    if (!QCoreApplication::instance())
        exit(1);

    QCoreApplication::exit();
}

int main(int argc, char** argv)
{
    if (!QFile::exists(LOCK_FILE))
    {
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


        /* here is the daemon part */
        struct sigaction action;

        action.sa_handler = signal_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGINT, &action, NULL);

        QFile file(LOCK_FILE);

        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << *argv << ": can't create lock file \"" << LOCK_FILE << "\".";
            return 1;
        }

        umask(0);
        file.close();
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
        DryingControl control("/etc/config.xml");

        app.exec();
        QFile::remove(PID_FILE);

        if (!QFile::remove(LOCK_FILE))
        {
            qDebug() << *argv << ": can't remove lock file \"" << LOCK_FILE << "\".";
            return 1;
        }

        return 0;
    }


    /* run as client */
    return 0;
}
