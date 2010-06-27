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
  qDebug() << command.property("name").toString();
}
