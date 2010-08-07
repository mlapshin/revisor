#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>
#include <QMouseEvent>
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

#define STRING_TO_ENUM(svar, evar, s, e) if (svar == s) { evar = e; }

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

Qt::MouseButton stringToMouseButton(const QString& button)
{
  Qt::MouseButton realButton = Qt::NoButton;

  STRING_TO_ENUM(button, realButton, "", Qt::NoButton);
  STRING_TO_ENUM(button, realButton, "left", Qt::LeftButton);
  STRING_TO_ENUM(button, realButton, "right", Qt::RightButton);
  STRING_TO_ENUM(button, realButton, "mid", Qt::MidButton);
  STRING_TO_ENUM(button, realButton, "xbutton1", Qt::XButton1);
  STRING_TO_ENUM(button, realButton, "xbutton2", Qt::XButton2);

  return realButton;
}

Qt::KeyboardModifier stringToModifier(const QString& modifier)
{
  Qt::KeyboardModifier ret = Qt::NoModifier;

  STRING_TO_ENUM(modifier, ret, "", Qt::NoModifier);
  STRING_TO_ENUM(modifier, ret, "shift", Qt::ShiftModifier);
  STRING_TO_ENUM(modifier, ret, "control", Qt::ControlModifier);
  STRING_TO_ENUM(modifier, ret, "ctrl", Qt::ControlModifier);
  STRING_TO_ENUM(modifier, ret, "alt", Qt::AltModifier);
  STRING_TO_ENUM(modifier, ret, "meta", Qt::MetaModifier);
  STRING_TO_ENUM(modifier, ret, "keypad", Qt::KeypadModifier);
  STRING_TO_ENUM(modifier, ret, "group_switch", Qt::GroupSwitchModifier);

  return ret;
}

DispatcherResponse Dispatcher::handleSessionTabSendMouseEventCommand(SessionTab* tab, const QScriptValue& command)
{
  DispatcherResponse response;
  QMouseEvent::Type realType;
  Qt::MouseButton realButton;
  Qt::MouseButtons realButtons;
  Qt::KeyboardModifiers realModifiers;

  assertParamPresent(command, "x");
  assertParamPresent(command, "y");
  ARG_FROM_COMMAND(unsigned int, x, "x", Number, 0);
  ARG_FROM_COMMAND(unsigned int, y, "y", Number, 0);

  ARG_FROM_COMMAND(QString, type, "type", String, "click");
  STRING_TO_ENUM(type, realType, "click", QEvent::MouseButtonPress);
  STRING_TO_ENUM(type, realType, "button_press", QEvent::MouseButtonPress);
  STRING_TO_ENUM(type, realType, "button_release", QEvent::MouseButtonRelease);
  STRING_TO_ENUM(type, realType, "dblclick", QEvent::MouseButtonDblClick);
  STRING_TO_ENUM(type, realType, "move", QEvent::MouseMove);

  ARG_FROM_COMMAND(QString, button, "button", String, "");
  realButton = stringToMouseButton(button);

  if ((realType == QEvent::MouseButtonPress ||
       realType == QEvent::MouseButtonRelease ||
       realType == QEvent::MouseButtonDblClick) &&
      realButton == Qt::NoButton) {
    throw Exception(QString("You should specify button for this type of mouse event"));
  }

  QScriptValue buttonsProperty = command.property("buttons");
  if (buttonsProperty.isArray()) {
    int arrayLength = buttonsProperty.property("length").toInt32();

    for (int i = 0; i < arrayLength; i++) {
      QScriptValue v = buttonsProperty.property(i);

      if (v.isString()) {
        realButtons |= stringToMouseButton(v.toString());
      }
    }
  }

  QScriptValue modifiersProperty = command.property("modifiers");
  if (modifiersProperty.isArray()) {
    int arrayLength = modifiersProperty.property("length").toInt32();

    for (int i = 0; i < arrayLength; i++) {
      QScriptValue v = modifiersProperty.property(i);

      if (v.isString()) {
        realModifiers |= stringToModifier(v.toString());
      }
    }
  }

  // TODO: this logic should be inside web page or tab, please refactor
  if (type == "click" || type == "dblclick") {
    tab->sendMouseEvent(QEvent::MouseButtonPress, QPoint(x, y), realButton, realButtons, realModifiers);
    tab->sendMouseEvent(QEvent::MouseButtonRelease, QPoint(x, y), realButton, realButtons, realModifiers);

    if (type == "dblclick") {
      tab->sendMouseEvent(QEvent::MouseButtonDblClick, QPoint(x, y), realButton, realButtons, realModifiers);
      tab->sendMouseEvent(QEvent::MouseButtonRelease, QPoint(x, y), realButton, realButtons, realModifiers);
    }
  } else {
    tab->sendMouseEvent(realType, QPoint(x, y), realButton, realButtons, realModifiers);
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
  Session* session = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_name").isValid()) {
    session = app->getSession(sessionName);
    tab = session->getTab(tabName);
    session->raiseTab(tabName);
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

  } else if (commandName == "session.tab.save_screenshot") {
    assertParamPresent(command, "file_name");
    ARG_FROM_COMMAND(QString, fileName, "file_name", String, "");
    ARG_FROM_COMMAND(unsigned int, vpWidth, "viewport_width", Number, 0);
    ARG_FROM_COMMAND(unsigned int, vpHeight, "viewport_height", Number, 0);

    tab->saveScreenshot(fileName, QSize(vpWidth, vpHeight));
  } else if (commandName == "session.tab.send_mouse_event") {
    return handleSessionTabSendMouseEventCommand(tab, command);
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
