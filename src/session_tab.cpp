#include "session_tab.hpp"
#include "session.hpp"
#include "counting_network_access_manager.hpp"
#include "web_page.hpp"
#include <QWebView>
#include <QVBoxLayout>
#include <QDebug>
#include <QWebFrame>
#include <QThread>

class SleepyThread : public QThread
{
 public:
  static void msleep(unsigned long msec) {
    QThread::msleep(msec);
  }
};

SessionTab::SessionTab(Session* s, const QString& n)
    : QObject(s), session(s), loadProgress(0), name(n)
{
  successfullLoad = false;
  networkManager = new CountingNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());

  webPage = new WebPage(this);
  webPage->setNetworkAccessManager(networkManager);

  QWidget* window = reinterpret_cast<QWidget*>(session->getTabWidget());
  webView = new QWebView(window);
  webView->setPage(webPage);
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
}

void SessionTab::visit(const QString& url)
{
  successfullLoad = false;
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

void SessionTab::setConfirmAnswer(bool a)
{
  webPage->setConfirmAnswer(a);
}

void SessionTab::setPromptAnswer(const QString& a, bool cancelled)
{
  webPage->setPromptAnswer(a, cancelled);
}

QVariant SessionTab::evaluateScript(const QString& script)
{
  return webView->page()->mainFrame()->evaluateJavaScript(script);
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
  successfullLoad = success;
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

  title += QString(" [%1]").arg(name);
  emit titleChanged(title);
}

void SessionTab::singleRequestFinished(QNetworkReply* reply)
{
  if (networkManager->getRequestsCount() == 0) {
    requestsFinished.wakeAll();
  }
}

void SessionTab::waitForAllRequestsFinished(unsigned int waitBefore, unsigned int waitAfter, unsigned int waitTimeout)
{
  bool finished = false;
  unsigned long timeout = waitTimeout == 0 ? ULONG_MAX : waitTimeout;

  SleepyThread::msleep(waitBefore);
  while (!finished) {
    requestsFinishedMutex.lock();
    requestsFinished.wait(&requestsFinishedMutex, timeout);
    requestsFinishedMutex.unlock();
    SleepyThread::msleep(waitAfter);

    if (networkManager->getRequestsCount() == 0) {
      finished = true;
    }
  }
}

bool SessionTab::waitForTrueEvaluation(const QString& script, unsigned int retryInterval, unsigned int tries)
{
  bool successfull = false;
  unsigned int currentTry = 0;

  while(!successfull && currentTry < tries) {
    QVariant result = evaluateScript(script);

    if (result.isValid() && result.type() == QVariant::Bool && result.toBool() == true) {
      successfull = true;
    }

    SleepyThread::msleep(retryInterval);
    currentTry++;
  }

  return successfull;
}

void SessionTab::saveScreenshot(const QString& fileName, const QSize& viewportSize)
{
  webPage->saveScreenshot(fileName, viewportSize);
}
