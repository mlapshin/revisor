#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>
#include "exception.hpp"
#include "json.hpp"

#define ARG_FROM_COMMAND(ctype, var, name, type, default) \
  ctype var = default; \
  if (command.property(name).isValid()) { \
    if (!command.property(name).is ## type ()) { \
      throw Exception("Parameter " name " should have type " #type); \
    } else { \
      var = command.property(name).to ## type (); \
    } \
  }

DeferredDispatcherResponseThread::DeferredDispatcherResponseThread(Dispatcher* d)
    : QThread(d), dispatcher(d)
{

}

class WaitForLoadThread : public DeferredDispatcherResponseThread
{
 public:
  WaitForLoadThread(Dispatcher* d, SessionTab* t, int to)
      : DeferredDispatcherResponseThread(d), tab(t), timeout(to) {}

  void run()
  {
    QTime t;
    t.start();
    bool timedOut = !tab->waitForLoad(timeout);
    bool successfull = tab->isLoadSuccessfull();

    response = JSON::response("OK", JSON::keyValue("timedOut", timedOut) + ", " + JSON::keyValue("elapsedTime", t.elapsed()) + ", " + JSON::keyValue("successfull", successfull));
  }

 private:
  SessionTab* tab;
  unsigned int timeout;
};

class WaitForAllRequestsFinishedThread : public DeferredDispatcherResponseThread
{
 public:
  WaitForAllRequestsFinishedThread(Dispatcher* d, SessionTab* t, int to, unsigned int b, unsigned int a)
      : DeferredDispatcherResponseThread(d), tab(t), timeout(to), waitBefore(b), waitAfter(a) {}

  void run()
  {
    QTime t;
    t.start();
    tab->waitForAllRequestsFinished(waitBefore, waitAfter, timeout);

    response = JSON::response("OK", JSON::keyValue("elapsedTime", t.elapsed()));
  }

 private:
  SessionTab* tab;
  unsigned int timeout;
  unsigned int waitBefore;
  unsigned int waitAfter;
};

Dispatcher::Dispatcher(Application* a)
    : QObject(a)
{
  app = a;
}

DispatcherResponse Dispatcher::handleSessionCommand(const QString& commandName, const QScriptValue& command)
{
  assertParamPresent(command, "session_name");

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
  assertParamPresent(command, "session_name");
  assertParamPresent(command, "tab_name");

  DispatcherResponse response;
  QString sessionName = command.property("session_name").toString();
  QString tabName     = command.property("tab_name").toString();
  SessionTab* tab     = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_name").isValid()) {
    tab = app->getSession(sessionName)->getTab(tabName);
  }

  if (commandName == "session.tab.create") {
    app->getSession(sessionName)->createTab(tabName);
  } else if (commandName == "session.tab.visit") {
    assertParamPresent(command, "url");
    QString url = command.property("url").toString();

    tab->visit(url);
  } else if (commandName == "session.tab.wait_for_load") {
    int timeout = 0;
    if (command.property("timeout").isValid()) {
      timeout = command.property("timeout").toInteger();
    }

    WaitForLoadThread* t = new WaitForLoadThread(this, tab, timeout);
    t->start();
    response.deferredThread = t;
  } else if (commandName == "session.tab.wait_for_all_requests_finished") {
    ARG_FROM_COMMAND(unsigned int, timeout, "timeout", Number, 0);
    ARG_FROM_COMMAND(unsigned int, waitBefore, "waitBefore", Number, 0);
    ARG_FROM_COMMAND(unsigned int, waitAfter, "waitAfter", Number, 0);

    WaitForAllRequestsFinishedThread* t = new WaitForAllRequestsFinishedThread(this, tab, timeout, waitBefore, waitAfter);
    t->start();
    response.deferredThread = t;
  } else if (commandName == "session.tab.evaluate_javascript") {
    assertParamPresent(command, "script");
    QString script = command.property("script").toString();
    QVariant res = tab->evaluateScript(script);

    response.response = JSON::response("OK", JSON::keyValue("evalResult", res));
  } else if (commandName == "session.tab.set_confirm_answer") {
    assertParamPresent(command, "answer");
    bool answer = command.property("answer").toBoolean();
    tab->setConfirmAnswer(answer);
  } else if (commandName == "session.tab.set_prompt_answer") {
    assertParamPresent(command, "cancelled");
    assertParamPresent(command, "answer");

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
  } catch (std::exception& e) {
    response.response = JSON::response("Error", JSON::keyValue("message", QString(e.what())));
  } catch (...) {
    response.response = JSON::response("Error", JSON::keyValue("message", "Unknown error"));
  }

  if (response.response.isEmpty()) {
    response.response = JSON::response("OK");
  }

  return response;
}

void Dispatcher::assertParamPresent(const QScriptValue& c, const QString& p)
{
  QScriptValue s = c.property(p);

  if (!s.isValid()) {
    throw Exception(QString("Parameter '%1' required").arg(p));
  }
}
