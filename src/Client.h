#ifndef __CLIENT__
#define __CLIENT__

#include "PipePublisher.h"
#include "PipeSubscriber.h"

#include <QObject>
#include <QTextStream>
#include <QSocketNotifier>

class Client : public QObject
{
    Q_OBJECT

public:
    Client(QObject* parent = 0);

    bool exec(void);

private slots:
    void messageReceived(const PipeSubscriber* pipe);
    void userInput(int descriptor);

private:

    enum Request {
        None,
        State
    };

    PipePublisher m_pipeOut;
    PipeSubscriber m_pipeIn;
    Request m_request;
//    QTextStream m_userIn;
    QTextStream m_userOut;
    QSocketNotifier m_stdinEvent;
};

#endif
