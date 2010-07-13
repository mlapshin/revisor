#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include <QNetworkCookieJar>
#include <QVBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QWebView>
#include <QDebug>
#include <QCloseEvent>

SessionWindow::SessionWindow(Session* s)
    : QWidget(), session(s)
{
  resize(800, 600);
  setWindowTitle(QString("%1 [Revisor]").arg(session->getName()));
}

void SessionWindow::closeEvent(QCloseEvent* event)
{
  event->ignore();
}

Session::Session(Application* a, const QString& n)
    : QObject(0)
{
  app = a;
  name = n;
  cookieJar = new QNetworkCookieJar(this);

  window = new SessionWindow(this);
  layout = new QVBoxLayout;
  tabWidget = new QTabWidget(window);
  window->setLayout(layout);
  layout->addWidget(tabWidget);
  window->show();
}

Session::~Session()
{
  window->close();
  delete window;
}

SessionTab* Session::createTab(const QString& tabName)
{
  SessionTab* s = new SessionTab(this, tabName);
  tabs.insert(tabName, s);
  tabWidget->addTab(s->getWebView(), QString("New Tab"));

  connect(s,    SIGNAL(titleChanged(const QString&)),
          this, SLOT(updateTabTitle(const QString&)));

  return s;
}

void Session::destroyTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    SessionTab* s = tabs[tabName];
    delete s;
    tabs.remove(tabName);
  }
}

void Session::updateTabTitle(const QString& newTitle)
{
  SessionTab* s = qobject_cast<SessionTab*>(sender());
  int idx = tabWidget->indexOf(s->getWebView());
  tabWidget->setTabText(idx, newTitle);
}

SessionTab* Session::getTab(const QString& tabName) const
{
  if (tabs.contains(tabName)) {
    return tabs[tabName];
  } else {
    return 0;
  }
}
