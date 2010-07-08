#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>

DeferredDispatcherResponseThread::DeferredDispatcherResponseThread(Dispatcher* d)
    : QThread(d), dispatcher(d)
{

}

class WaitForLoadThread : public DeferredDispatcherResponseThread
{
 public:
  WaitForLoadThread(Dispatcher* d, SessionTab* t)
      : DeferredDispatcherResponseThread(d), tab(t) {}
  void run();

 private:
  SessionTab* tab;
};

void WaitForLoadThread::run()
{
  QTime t;
  t.start();
  tab->waitForLoad();
  qDebug("Wait for load time: %.3f", t.elapsed() / 1000.0f);
}

Dispatcher::Dispatcher(Application* a)
    : QObject(a)
{
  app = a;
}

DispatcherResponse Dispatcher::handleSessionCommand(const QString& commandName, const QScriptValue& command)
{
  DispatcherResponse response;
  QString sessionName = command.property("session_name").toString();

  if (commandName == "session.start") {
    app->startSession(sessionName);
  } else if (commandName == "session.stop") {
    app->stopSession(sessionName);
  }

  return response;
}

DispatcherResponse Dispatcher::handleSessionTabCommand(const QString& commandName, const QScriptValue& command)
{
  DispatcherResponse response;
  QString sessionName = command.property("session_name").toString();
  int tabIndex        = command.property("tab_index").toInteger();
  QString url         = command.property("url").toString();
  SessionTab* tab     = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_index").isValid()) {
    tab = app->getSession(sessionName)->getTab(tabIndex);
  }

  if (commandName == "session.tab.create") {
    app->getSession(sessionName)->createTab();
  } else if (commandName == "session.tab.visit") {
    tab->visit(url);
  } else if (commandName == "session.tab.wait_for_load") {
    WaitForLoadThread* t = new WaitForLoadThread(this, tab);
    t->start();
    response.deferredThread = t;
  } else if (commandName == "session.tab.evaluate_script") {
    QString script = command.property("script").toString();
    response.response = tab->evaluateScript(script).toString();
  }

  return response;
}

DispatcherResponse Dispatcher::dispatch(const QScriptValue& command)
{
  DispatcherResponse response;
  QString commandName = command.property("name").toString();
  QStringList parsedCommandName = commandName.split(".");
  qDebug() << "Received command " << commandName;

  if (parsedCommandName[0] == "session") {
    if (parsedCommandName[1] == "tab") {
      return handleSessionTabCommand(commandName, command);
    } else {
      return handleSessionCommand(commandName, command);
    }
  }

  return response;
}
