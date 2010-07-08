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

SessionWindow::SessionWindow()
    : QWidget()
{
  resize(800, 600);
  setWindowTitle("Revisor Session");
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

  window = new SessionWindow();
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

unsigned int Session::createTab()
{
  SessionTab* s = new SessionTab(this);
  tabs.append(s);
  tabWidget->addTab(s->getWebView(), QString("New Tab"));

  connect(s,    SIGNAL(titleChanged(const QString&)),
          this, SLOT(updateTabTitle(const QString&)));

  return tabs.length() - 1;
}

void Session::updateTabTitle(const QString& newTitle)
{
  // TODO: implemet me without sender() function
  SessionTab* s = qobject_cast<SessionTab*>(sender());
  int idx = tabs.indexOf(s);
  tabWidget->setTabText(idx, newTitle);
}

SessionTab* Session::getTab(int idx) const
{
  if (idx < tabs.length()) {
    return tabs[idx];
  } else {
    return 0;
  }
}
