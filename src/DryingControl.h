#ifndef __DRYING_CONTROL__
#define __DRYING_CONTROL__

#include "PipePublisher.h"
#include "PipeSubscriber.h"

#include <QObject>
#include <QVector>
#include <QByteArray>

class QDomNode;
class Port;
class AlertHandler;
class TempSensor;

class DryingControl : public QObject
{
    Q_OBJECT

public:

    enum Command {
        None = 0,
        Exit
    };

    DryingControl(const QByteArray& configFile = QByteArray("/etc/dryingC/config.xml"));
    ~DryingControl(void);

private slots:
    void smokeAlarmStateChanged(bool value);
    void messageReceived(const PipeSubscriber* pipe);

private:
    void configureDigitalIO(const QDomNode& node);
    void configureTempSensors(const QDomNode& node);
    Port* getPortByName(const QByteArray& portName);
    void sendStateThrowPipe(void);

    QVector<Port*> m_digitalIO;
    QVector<TempSensor*> m_temperatures;
    AlertHandler* m_alertHandler;
    PipeSubscriber m_pipeIn;
    PipePublisher m_pipeOut;
};

#endif
