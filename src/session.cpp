#include "session.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include <QNetworkCookieJar>
#include <QVBoxLayout>
#include <QWidget>

Session::Session(Application* a)
    : QObject(0)
{
  app = a;
  cookieJar = new QNetworkCookieJar(this);

  window = new QWidget();
  window->setWindowTitle("Session Window [Revisor]");
  QVBoxLayout* layout = new QVBoxLayout;

  window->setLayout(layout);
  window->show();

  defaultTab = new SessionTab(this);
}

Session::~Session()
{
  window->close();
  delete window;
}
