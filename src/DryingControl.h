#ifndef __DRYING_CONTROL__
#define __DRYING_CONTROL__

#include <QObject>
#include <QVector>
#include <QByteArray>

class QDomNode;
class Port;
class AlertHandler;

class DryingControl : public QObject
{
    Q_OBJECT

public:
    DryingControl(const QByteArray& configFile = QByteArray("config.xml"));
    ~DryingControl(void);

private slots:
    void smokeAlarmStateChanged(bool value);

private:
    void configureDigitalIO(const QDomNode& node);
    Port* getPortByName(const QByteArray& portName);

    QVector<Port*> m_digitalIO;
    AlertHandler* m_alertHandler;
};

#endif
