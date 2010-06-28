#include "dispatcher.hpp"
#include "application.hpp"
#include <QDebug>
#include <QScriptValueIterator>

Dispatcher::Dispatcher(Application* a)
    : QObject(a)
{
  app = a;
}

void Dispatcher::dispatch(const QScriptValue& command)
{
  QString commandName = command.property("name").toString();

  if (commandName == "session.start") {
    app->startSession();
  } else if (commandName == "session.stop") {
    int sessionIndex = command.property("index").toInteger();
    app->stopSession(sessionIndex);
  }
}