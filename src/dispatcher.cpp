#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>
#include "exception.hpp"
#include "json.hpp"

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
  bool timedOut = !tab->waitForLoad(timeout);
  response = JSON::response("OK", JSON::keyValue("timedOut", timedOut) + ", " + JSON::keyValue("elapsedTime", t.elapsed()));
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
    QVariant res = tab->evaluateScript(script);

    response.response = JSON::response("OK", JSON::keyValue("evalResult", res));
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

  try {
    QString commandName = command.property("name").toString();
    QStringList parsedCommandName = commandName.split(".");

    if (parsedCommandName[0] == "session") {
      if (parsedCommandName[1] == "tab") {
        response = handleSessionTabCommand(commandName, command);
      } else {
        response = handleSessionCommand(commandName, command);
      }
    }
  } catch (Exception& e) {
    response.response = JSON::response("Error", JSON::keyValue("message", e.getMessage()));
  }

  if (response.response.isEmpty()) {
    response.response = JSON::response("OK");
  }

  return response;
}
