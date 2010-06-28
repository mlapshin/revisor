#include "session.hpp"
#include "application.hpp"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QWebView>
#include <QVBoxLayout>

Session::Session(Application* a)
    : QObject(0)
{
  app = a;
  networkManager = new QNetworkAccessManager(this);
  cookieJar      = new QNetworkCookieJar(networkManager);
  networkManager->setCookieJar(cookieJar);

  window = new QWidget();
  window->setWindowTitle("Session Window [Revisor]");
  QVBoxLayout* layout = new QVBoxLayout;

  webView = new QWebView(window);
  webView->page()->setNetworkAccessManager(networkManager);
  layout->addWidget(webView);

  window->setLayout(layout);
  window->show();
  webView->load(QString("http://google.com/"));
}

Session::~Session()
{
  window->close();
  delete window;
}
