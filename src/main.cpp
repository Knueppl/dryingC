#include "IOHandler.h"
#include "DryingControl.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <unistd.h>
#include <sys/types.h>

const char* LOCK_FILE = "/etc/dryingC/daemon.lock";

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

        QFile file(LOCK_FILE);

        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << *argv << ": can't create lock file \"" << LOCK_FILE << "\".";
            return 1;
        }

        file.close();

        /* start daemon process */
        QCoreApplication app(argc, argv);
        DryingControl control;

        app.exec();

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
