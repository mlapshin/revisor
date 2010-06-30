#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include <QNetworkCookieJar>
#include <QVBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QWebView>
#include <QDebug>

Session::Session(Application* a)
    : QObject(0)
{
  app = a;
  cookieJar = new QNetworkCookieJar(this);

  window = new QWidget();
  window->resize(QSize(800, 600));
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
  tabWidget->addTab(s->getWebView(), QString("New Tab"));

  connect(s,    SIGNAL(titleChanged(const QString&)),
          this, SLOT(updateTabTitle(const QString&)));
}

void Session::updateTabTitle(const QString& newTitle)
{
  // TODO: implemet me without sender() function
  SessionTab* s = qobject_cast<SessionTab*>(sender());
  int idx = tabs.indexOf(s);
  tabWidget->setTabText(idx, newTitle);
}

SessionTab* Session::getTab(unsigned int idx) const
{
  if (idx < tabs.length()) {
    return tabs[idx];
  } else {
    return 0;
  }
}
