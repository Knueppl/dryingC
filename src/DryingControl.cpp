#include "DryingControl.h"
#include "IOHandler.h"
#include "AlertHandler.h"
#include "PortFactory.h"
#include "Port.h"

#include <QDomNode>

DryingControl::DryingControl(const QByteArray& configFile)
    : m_alertHandler(0)
{
    QFile file(configFile);
    QDomDocument doc;

    if (!file.open(QIODevice::ReadOnly))
    {
        io << "DryingControl: can't open file \"" << configFile << "\".\n";
        return;
    }

    if (!doc.setContent(&file))
    {
        io << "DryingControl: QDomDocument error.\n";
        file.close();
        return;
    }

    file.close();
    const QDomElement root = doc.firstChild().toElement();

    if (root.isNull() || root.tagName() != "drying_control")
        return;

    QDomNodeList nodes(root.childNodes());

    for (int i = 0; i < nodes.size(); i++)
    {
        const QDomElement tag(nodes.at(i).toElement());

        if (tag.isNull())
            continue;

        if (tag.tagName() == "digital_io")
            this->configureDigitalIO(tag);
        else if (tag.tagName() == "alert_handler")
            m_alertHandler = new AlertHandler(tag);
    }

    Port* smokeAlarm(this->getPortByName("Feuermelder"));

    if (!m_alertHandler || !smokeAlarm)
        return;

    this->connect(smokeAlarm, SIGNAL(valueChanged(bool)), this, SLOT(smokeAlarmStateChanged(bool)));
}

DryingControl::~DryingControl(void)
{
    delete m_alertHandler;
    qDeleteAll(m_digitalIO);
}

void DryingControl::configureDigitalIO(const QDomNode& node)
{
    const QDomElement root(node.toElement());

    if (root.isNull())
    {
        io << "DryingControl: durring configure digital io. Node isn't a element.\n";
        return;
    }

    QDomNodeList nodes(root.childNodes());
    PortFactory factory("/sys/kernel/debug/omap_mux/", "/sys/class/gpio/");

    for (int i = 0; i < nodes.size(); i++)
    {
        const QDomElement tag(nodes.at(i).toElement());

        if (tag.isNull() || tag.tagName() != "port")
            continue;

        Port* port(factory.build(tag));

        if (!port)
            continue;

        m_digitalIO.push_back(port);
    }
}

Port* DryingControl::getPortByName(const QByteArray& portName)
{
    for (QVector<Port*>::iterator it(m_digitalIO.begin()); it < m_digitalIO.end(); ++it)
        if ((*it)->portName() == portName)
            return *it;

    return 0;
}

void DryingControl::smokeAlarmStateChanged(bool value)
{
    static bool alarmOn = false;

    io << "DryingControl: smoke alarm state changed. New satate = " << value << ".\n";

    if (alarmOn || value)
        return;

    if (!m_alertHandler)
    {
        io << "DryingControl: alert handler not valid.\n";
        return;
    }

    m_alertHandler->startAlertRoutine();
}
