#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>

class WaitForLoadThread : public QThread
{
 public:
  WaitForLoadThread(int s, int t, Application* a) : sessionIndex(s), tabIndex(t), app(a) {}
  void run();

 private:
  int sessionIndex;
  int tabIndex;
  Application* app;
};

void WaitForLoadThread::run()
{
  app->getSession(sessionIndex)->getTab(tabIndex)->waitForLoad();
}

Dispatcher::Dispatcher(Application* a)
    : QObject(a)
{
  app = a;
}

DispatcherResponse Dispatcher::dispatch(const QScriptValue& command)
{
  QString commandName = command.property("name").toString();

  if (commandName == "session.start") {
    app->startSession();

  } else if (commandName == "session.stop") {
    int sessionIndex = command.property("session_index").toInteger();
    app->stopSession(sessionIndex);

  } else if (commandName == "session.tab.create") {
    int sessionIndex = command.property("session_index").toInteger();
    app->getSession(sessionIndex)->createTab();

  } else if (commandName == "session.tab.visit") {
    int sessionIndex = command.property("session_index").toInteger();
    int tabIndex     = command.property("tab_index").toInteger();
    QString url      = command.property("url").toString();
    app->getSession(sessionIndex)->getTab(tabIndex)->visit(url);

  } else if (commandName == "session.tab.wait_for_load") {
    int sessionIndex = command.property("session_index").toInteger();
    int tabIndex     = command.property("tab_index").toInteger();

    WaitForLoadThread t(sessionIndex, tabIndex, app);
    t.start();
  }
}
