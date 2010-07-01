#include "session_tab.hpp"
#include "session.hpp"
#include "counting_network_access_manager.hpp"
#include <QWebView>
#include <QVBoxLayout>
#include <QDebug>

SessionTab::SessionTab(Session* s)
    : QObject(s), session(s), loadProgress(0)
{
  networkManager = new CountingNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());

  QWidget* window = reinterpret_cast<QWidget*>(session->getTabWidget());
  webView = new QWebView(window);
  webView->page()->setNetworkAccessManager(networkManager);
  webView->setDisabled(true);

  connect(webView, SIGNAL(titleChanged(const QString&)),
          this,    SLOT(updateTitle(const QString&)));

  connect(webView, SIGNAL(loadProgress(int)),
          this,    SLOT(updateProgress(int)));

  connect(webView, SIGNAL(loadFinished(bool)),
          this,    SLOT(loadFinished(bool)));

  connect(networkManager, SIGNAL(finished(QNetworkReply*)),
          this,           SLOT(singleRequestFinished(QNetworkReply*)));
}

SessionTab::~SessionTab()
{
  qDebug() << "SessionTab::~SessionTab()";
}

void SessionTab::visit(const QString& url)
{
  webView->load(url);
}

bool SessionTab::waitForLoad(unsigned int t)
{
  unsigned long timeout = t;

  if (t == 0) {
    timeout = ULONG_MAX;
  }

  pageLoadedMutex.lock();
  bool result = pageLoaded.wait(&pageLoadedMutex, timeout);
  pageLoadedMutex.unlock();

  return result;
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

void SessionTab::loadFinished(bool success)
{
  loadProgress = 100;
  _updateTabTitle();

  if (networkManager->getRequestsCount() == 0) {
    pageLoaded.wakeAll();
  }
}

void SessionTab::_updateTabTitle()
{
  QString title = pageTitle;

  if (loadProgress < 100) {
    title += QString(" (%1%)").arg(loadProgress);
  }

  titleChanged(title);
}

void SessionTab::singleRequestFinished(QNetworkReply* reply)
{
}
