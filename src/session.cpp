#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "exception.hpp"
#include "main_window.hpp"
#include <QNetworkCookieJar>
#include <QDebug>

Session::Session(Application* a, const QString& n)
    : QObject(a)
{
  app = a;
  name = n;
  cookieJar = new QNetworkCookieJar(this);
}

Session::~Session()
{
  // Collect all tab names to list and then destroy each tab personally
  QStringList tabNames;
  for (SessionTabsMap::Iterator it = tabs.begin(); it != tabs.end(); it++) {
    tabNames.append(it.key());
  }

  for (QStringList::Iterator it = tabNames.begin(); it != tabNames.end(); it++) {
    destroyTab(*it);
  }
}

SessionTab* Session::createTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    throw Exception(QString("Session tab with name '%1' already exists in session '%2'").arg(name).arg(name));
  } else {
    SessionTab* s = new SessionTab(this, tabName);
    tabs.insert(tabName, s);
    app->getMainWindow()->registerSessionTab(s);

    return s;
  }
}

void Session::destroyTab(const QString& tabName)
{
  if (tabs.contains(tabName)) {
    SessionTab* s = tabs.value(tabName);
    app->getMainWindow()->unregisterSessionTab(s);

    delete s;
    tabs.remove(tabName);
  } else {
    throw Exception(QString("Session tab with name '%1' does not exists in session '%2'").arg(tabName).arg(name));
  }
}

SessionTab* Session::getTab(const QString& tabName) const
{
  if (tabs.contains(tabName)) {
    return tabs.value(tabName);
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
