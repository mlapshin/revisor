#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "exception.hpp"
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
    : QObject(a)
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
  for (SessionTabsMap::Iterator it = tabs.begin(); it != tabs.end(); it++) {
    delete it.value();
  }

  window->close();
  delete window;
}

SessionTab* Session::createTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    throw Exception(QString("Session tab with name '%1' already exists in session '%2'").arg(name).arg(name));
  } else {
    SessionTab* s = new SessionTab(this, tabName);
    tabs.insert(tabName, s);
    tabWidget->addTab(s->getWebView(), QString("New Tab"));

    connect(s,    SIGNAL(titleChanged(const QString&)),
            this, SLOT(updateTabTitle(const QString&)));

    return s;
  }
}

void Session::destroyTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    SessionTab* s = tabs[tabName];
    delete s;
    tabs.remove(tabName);
  } else {
    throw Exception(QString("Session tab with name '%1' does not exists in session '%2'").arg(tabName).arg(name));
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

void Session::setCookiesFor(const QList<QNetworkCookie>& cookies, const QUrl& from)
{
  cookieJar->setCookiesFromUrl(cookies, from);
}

QList<QNetworkCookie> Session::getCookiesFor(const QUrl& url) const
{
  return cookieJar->cookiesForUrl(url);
}

void Session::raiseTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    window->raise();
    tabWidget->setCurrentIndex(getTabIndex(tabName));
  }
}

void Session::raiseTab(SessionTab* t)
{
  raiseTab(t->getName());
}

int Session::getTabIndex(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    return tabWidget->indexOf(tabs[tabName]->getWebView());
  } else {
    return -1;
  }
}
