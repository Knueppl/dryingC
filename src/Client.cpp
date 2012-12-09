#include "Client.h"

#include <QCoreApplication>
#include <QDebug>
#include <unistd.h>
#include <cstdio>

namespace {
const char* MSG_HELLO = "dyingC client version 0.0.1\n"
                        "...\n\n\n";
const char* MSG_MENU = "(a) print currnt state\n"
                       "(q) quit\n"
                       "--------------------------\n"
                       "you want? ";
}

Client::Client(QObject* parent)
    : QObject(parent),
      m_pipeOut("/etc/dryingC/pipe-out.key"),
      m_pipeIn("/etc/dryingC/pipe-in.key"),
      m_request(None),
//      m_userIn(stdin),
      m_userOut(stdout, QIODevice::WriteOnly),
      m_stdinEvent(STDIN_FILENO, QSocketNotifier::Read)
{
    this->connect(&m_pipeIn, SIGNAL(messageReceived(const PipeSubscriber*)), this, SLOT(messageReceived(const PipeSubscriber*)));
    this->connect(&m_stdinEvent, SIGNAL(activated(int)), this, SLOT(userInput(int)));

    m_userOut << MSG_HELLO << MSG_MENU;
    m_userOut.flush();
}

void Client::messageReceived(const PipeSubscriber* pipe)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (pipe->isNull())
        return;

    switch (m_request)
    {
    case State:
        if (pipe->isText())
        {
            m_userOut << pipe->text();
            m_userOut.flush();
        }
        break;

    default:
        break;
    }
}

void Client::userInput(int)
{
    QByteArray in;
    QTextStream userIn(stdin, QIODevice::ReadOnly);

    userIn >> in;
    qDebug() << __PRETTY_FUNCTION__ << "in = " << in;

    if (in == "q")
    {
        if (QCoreApplication::instance()) QCoreApplication::instance()->exit();
    }
    else if (in == "a")
    {
        m_pipeOut.send("state");
        m_request = State;
    }
    else
    {
        m_userOut << MSG_MENU;
        m_userOut.flush();
    }
}
