#include "IOHandler.h"
#include "AlertHandler.h"

#include <QCoreApplication>
#include <QDomNode>
#include <QFile>
#include <QDomDocument>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QObject::connect(&io, SIGNAL(finished()), &app, SLOT(quit()));

    /* Start io-handler thread */
    io.start();


    AlertHandler alert(root);
    alert.startAlertRoutine();

    return app.exec();
}
