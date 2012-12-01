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

    QFile file("config.xml");
    QDomDocument doc;

    if (!file.open(QIODevice::ReadOnly))
    {
        return 1;
    }

    if (!doc.setContent(&file))
    {
        file.close();
        return 1;
    }

    file.close();
    QDomElement root = doc.firstChild().toElement();

    if (root.isNull() || root.tagName() != "alert_handler")
        return 1;

    AlertHandler alert(root);
    alert.startAlertRoutine();

    return app.exec();
}
