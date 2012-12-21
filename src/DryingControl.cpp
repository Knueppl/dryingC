#include "DryingControl.h"
#include "IOHandler.h"
#include "AlertHandler.h"
#include "PortFactory.h"
#include "Port.h"
#include "STLM75.h"

#include <QCoreApplication>
#include <QDomNode>
#include <QDebug>

#define MSG(x) (io() << "DryingControl: " << x << ".")

DryingControl::DryingControl(const QByteArray& configFile)
    : m_alertHandler(0),
      m_pipeIn("/etc/dryingC/pipe-out.key"),
      m_pipeOut("/etc/dryingC/pipe-in.key")
{
    this->connect(&m_pipeIn, SIGNAL(messageReceived(const PipeSubscriber*)), this, SLOT(messageReceived(const PipeSubscriber*)));

    QFile file(configFile);
    QDomDocument doc;

    if (!file.open(QIODevice::ReadOnly))
    {
        io() << "DryingControl: can't open file \"" << configFile << "\".";
        return;
    }

    if (!doc.setContent(&file))
    {
        io() << "DryingControl: QDomDocument error.";
        file.close();
        return;
    }

    file.close();
    const QDomElement root = doc.firstChild().toElement();

    if (root.isNull() || root.tagName() != "drying_control")
    {
        io() << "DryingControl: wrong tag.";
        return;
    }

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
        else if (tag.tagName() == "temp_sensor")
            this->configureTempSensors(tag);
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
    qDeleteAll(m_temperatures);
}

void DryingControl::configureDigitalIO(const QDomNode& node)
{
    io() << "DryingControl: configure digital ios.";
    const QDomElement root(node.toElement());

    if (root.isNull())
    {
        io() << "DryingControl: durring configure digital io. Node isn't a element.";
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

void DryingControl::configureTempSensors(const QDomNode& node)
{
    MSG("configure temperature sensors");
    const QDomNode root(node.toElement());

    if (root.isNull())
    {
        MSG("xml node not valid");
        return;
    }

    QDomNodeList nodes(root.childNodes());

    for (int i = 0; i < nodes.size(); i++)
    {
        const QDomElement tag(nodes.at(i).toElement());

        if (tag.isNull())
            continue;

        if (tag.tagName() == "stlm75")
            m_temperatures.push_back(new STLM75(tag));
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

    io() << "DryingControl: smoke alarm state changed. New satate = " << value << ".";

    if (alarmOn || value)
        return;

    if (!m_alertHandler)
    {
        io() << "DryingControl: alert handler not valid.";
        return;
    }

    m_alertHandler->startAlertRoutine();
}

void DryingControl::messageReceived(const PipeSubscriber* pipe)
{
    if (pipe->isNull())
    {
        MSG("messageReceived, pipe is null");
        return;
    }

    if (pipe->isText())
    {
        const QByteArray msg(pipe->text());

        if (msg == "state")
            this->sendStateThrowPipe();

        /* for future */
        return;
    }
    else if (pipe->isCommand())
    {
        switch (pipe->command())
        {
        case Exit:
            if (QCoreApplication::instance())
            {
                m_pipeOut.send("dryingC daemon is shuting down...");
                QCoreApplication::instance()->exit();
            }
            break;

        default:
            MSG("unkown command");
            break;
        }
    }
}

namespace {
const char* MSG_STATE_DIGPORTS = "###############################\n"
                                 "# Digitalports State          #\n"
                                 "###############################\n";

const char* MSG_ALERTHANDLER = "###############################\n"
                               "# Alert                       #\n"
                               "###############################\n";

const char* MSG_STATE_TEMPERATURE = "###############################\n"
                                    "# Temperature Sensors         #\n"
                                    "###############################\n";
}

void DryingControl::sendStateThrowPipe(void)
{
    qDebug() << __PRETTY_FUNCTION__;
    QByteArray msg;
    QTextStream stream(&msg, QIODevice::WriteOnly);

    stream << MSG_STATE_DIGPORTS;

    for (QVector<Port*>::iterator port = m_digitalIO.begin(); port < m_digitalIO.end(); ++port)
        stream << **port;

    if (m_alertHandler)
    {
        stream << MSG_ALERTHANDLER;
        stream << *m_alertHandler;
    }

    stream << MSG_STATE_TEMPERATURE;

    for (QVector<TempSensor*>::iterator sensor = m_temperatures.begin(); sensor < m_temperatures.end(); ++sensor)
    {
        (**sensor).grab();
        stream << **sensor;
    }

    stream.flush();
    m_pipeOut.send(msg);
}
