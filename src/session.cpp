#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include <QNetworkCookieJar>
#include <QVBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QWebView>

Session::Session(Application* a)
    : QObject(0)
{
  app = a;
  cookieJar = new QNetworkCookieJar(this);

  window = new QWidget();
  window->setWindowTitle("Session Window [Revisor]");
  layout = new QVBoxLayout;
  tabWidget = new QTabWidget(window);
  window->setLayout(layout);
  layout->addWidget(tabWidget);
  window->show();

  createTab();
}

Session::~Session()
{
  window->close();
  delete window;
}

void Session::createTab()
{
  SessionTab* s = new SessionTab(this);
  tabs.append(s);

  tabWidget->addTab(s->getWebView(), QString("Tab"));
}
