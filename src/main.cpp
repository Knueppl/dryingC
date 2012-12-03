#include "IOHandler.h"
#include "DryingControl.h"

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

    DryingControl control();

    return app.exec();
}
