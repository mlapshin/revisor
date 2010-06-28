#include "session_tab.hpp"
#include "session.hpp"
#include <QNetworkAccessManager>
#include <QWebView>
#include <QVBoxLayout>
#include <QDebug>

SessionTab::SessionTab(Session* s)
    : QObject(s), session(s)
{
  networkManager = new QNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());

  QWidget* window = session->getWindow();
  webView = new QWebView(window);
  webView->page()->setNetworkAccessManager(networkManager);
  window->layout()->addWidget(webView);

  webView->load(QString("http://google.com/"));
}

SessionTab::~SessionTab()
{
  qDebug() << "SessionTab::~SessionTab()";
}
