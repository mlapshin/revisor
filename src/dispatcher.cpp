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
  WaitForLoadThread(Dispatcher* d, SessionTab* t, int to)
      : DeferredDispatcherResponseThread(d), tab(t), timeout(to) {}
  void run();

 private:
  SessionTab* tab;
  unsigned int timeout;
};

void WaitForLoadThread::run()
{
  QTime t;
  t.start();
  tab->waitForLoad(timeout);
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
  QString tabName     = command.property("tab_name").toString();
  QString url         = command.property("url").toString();
  SessionTab* tab     = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_name").isValid()) {
    tab = app->getSession(sessionName)->getTab(tabName);
  }

  if (commandName == "session.tab.create") {
    app->getSession(sessionName)->createTab(tabName);
  } else if (commandName == "session.tab.visit") {
    tab->visit(url);
  } else if (commandName == "session.tab.wait_for_load") {
    int timeout = 0;
    if (command.property("timeout").isValid()) {
      timeout = command.property("timeout").toInteger();
    }

    WaitForLoadThread* t = new WaitForLoadThread(this, tab, timeout);
    t->start();
    response.deferredThread = t;
  } else if (commandName == "session.tab.evaluate_javascript") {
    QString script = command.property("script").toString();
    response.response = tab->evaluateScript(script).toString();
  } else if (commandName == "session.tab.set_confirm_answer") {
    bool answer = command.property("answer").toBoolean();
    tab->setConfirmAnswer(answer);
  } else if (commandName == "session.tab.set_prompt_answer") {
    bool cancelled = command.property("cancelled").toBoolean();
    QString answer = command.property("answer").toString();
    tab->setPromptAnswer(answer, cancelled);
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
