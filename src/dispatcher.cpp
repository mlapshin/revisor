#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>
#include "exception.hpp"
#include "json.hpp"

#define ARG_FROM_COMMAND(ctype, var, name, type, default)            \
  ctype var = default;                                               \
  if (command.property(name).isValid()) {                            \
    if (!command.property(name).is ## type ()) {                     \
      throw Exception("Parameter " name " should have type " #type); \
    } else {                                                         \
      var = command.property(name).to ## type ();                    \
    }                                                                \
  }

#define COOKIE_PARAM_WITH_CAST(name, pname, type, cast, required)       \
  if (it.value().property(name).is ## type ()) {                        \
    cookie.set ## pname (it.value().property(name).to ## type ().to ## cast ()); \
  } else if (required) {                                                \
    throw Exception("Cookie parameter " name " is required");           \
  }

#define COOKIE_PARAM(name, pname, type, required)                       \
  if (it.value().property(name).is ## type ()) {                        \
    cookie.set ## pname (it.value().property(name).to ## type ());      \
  } else if (required) {                                                \
    throw Exception("Cookie parameter " name " is required");           \
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

    response = JSON::response("OK", JSON::keyValue("timed_out", timedOut) + ", " + JSON::keyValue("elapsed_time", t.elapsed()) + ", " + JSON::keyValue("successfull", successfull));
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

    response = JSON::response("OK", JSON::keyValue("elapsed_time", t.elapsed()));
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
  DispatcherResponse response;
  assertParamPresent(command, "session_name");
  ARG_FROM_COMMAND(QString, sessionName, "session_name", String, "");

  if (commandName == "session.start") {
    app->startSession(sessionName);
  } else if (commandName == "session.stop") {
    app->stopSession(sessionName);
  } else if (commandName == "session.set_cookies") {
    QList<QNetworkCookie> cookies;
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");

    if (command.property("cookies").isArray()) {
      QScriptValueIterator it(command.property("cookies"));

      while(it.hasNext()) {
        it.next();
        QNetworkCookie cookie;

        // TODO: rewrite me in more generalized way
        COOKIE_PARAM_WITH_CAST("name", Name, String, Utf8, true);
        COOKIE_PARAM_WITH_CAST("value", Value, String, Utf8, true);
        COOKIE_PARAM_WITH_CAST("path", Path, String, Utf8, false);
        COOKIE_PARAM_WITH_CAST("domain", Domain, String, Utf8, false);
        COOKIE_PARAM("http_only", HttpOnly, Bool, false);
        COOKIE_PARAM("secure", Secure, Bool, false);
        if (it.value().property("expires_at").isValid()) {
          QDateTime expiresAt = QDateTime::fromString(it.value().property("expires_at").toString(), Qt::ISODate);
          expiresAt.setTimeSpec(Qt::UTC);
          cookie.setExpirationDate(expiresAt);
        }

        cookies.append(cookie);
      }
    } else {
      throw Exception("Argument 'cookies' of command 'session.set_cookies' must be an array");
    }

    app->getSession(sessionName)->setCookiesFor(cookies, url);
  } else if (commandName == "session.get_cookies") {
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");

    QList<QNetworkCookie> cookies = app->getSession(sessionName)->getCookiesFor(url);
    response.response = JSON::response("OK", JSON::keyValue("cookies", cookies));
  }

  return response;
}

DispatcherResponse Dispatcher::handleSessionTabCommand(const QString& commandName, const QScriptValue& command)
{
  assertParamPresent(command, "session_name");
  assertParamPresent(command, "tab_name");

  DispatcherResponse response;
  ARG_FROM_COMMAND(QString, sessionName, "session_name", String, "");
  ARG_FROM_COMMAND(QString, tabName, "tab_name", String, "");
  SessionTab* tab = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_name").isValid()) {
    tab = app->getSession(sessionName)->getTab(tabName);
  }

  if (commandName == "session.tab.create") {
    app->getSession(sessionName)->createTab(tabName);
  } else if (commandName == "session.tab.destroy") {
    app->getSession(sessionName)->destroyTab(tabName);
  } else if (commandName == "session.tab.visit") {
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");
    tab->visit(url);

  } else if (commandName == "session.tab.wait_for_load") {
    ARG_FROM_COMMAND(unsigned int, timeout, "timeout", Number, 0);

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
    ARG_FROM_COMMAND(QString, script, "script", String, "");

    QVariant res = tab->evaluateScript(script);
    response.response = JSON::response("OK", JSON::keyValue("eval_result", res));

  } else if (commandName == "session.tab.set_confirm_answer") {
    assertParamPresent(command, "answer");
    ARG_FROM_COMMAND(bool, answer, "answer", Boolean, false);
    tab->setConfirmAnswer(answer);

  } else if (commandName == "session.tab.set_prompt_answer") {
    assertParamPresent(command, "cancelled");
    assertParamPresent(command, "answer");
    ARG_FROM_COMMAND(bool, cancelled, "cancelled", Boolean, false);
    ARG_FROM_COMMAND(QString, answer, "answer", String, "");

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
