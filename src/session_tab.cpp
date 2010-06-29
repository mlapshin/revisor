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

  QWidget* window = reinterpret_cast<QWidget*>(session->getTabWidget());
  webView = new QWebView(window);
  webView->page()->setNetworkAccessManager(networkManager);
  webView->load(QString("http://google.com/"));
}

SessionTab::~SessionTab()
{
  qDebug() << "SessionTab::~SessionTab()";
}
