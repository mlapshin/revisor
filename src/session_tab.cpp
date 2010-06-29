#include "session_tab.hpp"
#include "session.hpp"
#include <QNetworkAccessManager>
#include <QWebView>
#include <QVBoxLayout>
#include <QDebug>

SessionTab::SessionTab(Session* s)
    : QObject(s), session(s), loadProgress(0)
{
  networkManager = new QNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());

  QWidget* window = reinterpret_cast<QWidget*>(session->getTabWidget());
  webView = new QWebView(window);
  webView->page()->setNetworkAccessManager(networkManager);
  webView->load(QString("http://google.com/"));

  connect(webView, SIGNAL(titleChanged(const QString&)),
          this,    SLOT(updateTitle(const QString&)));

  connect(webView, SIGNAL(loadProgress(int)),
          this,    SLOT(updateProgress(int)));
}

SessionTab::~SessionTab()
{
  qDebug() << "SessionTab::~SessionTab()";
}

void SessionTab::updateTitle(const QString& t)
{
  pageTitle = t;
  _updateTabTitle();
}

void SessionTab::updateProgress(int p)
{
  loadProgress = p;
  _updateTabTitle();
}

void SessionTab::_updateTabTitle()
{
  QString title = pageTitle;

  if (loadProgress < 100) {
    title += QString(" (%1%)").arg(loadProgress);
  }

  titleChanged(title);
}
