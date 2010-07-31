#include "session_tab.hpp"
#include "session.hpp"
#include "counting_network_access_manager.hpp"
#include "web_page.hpp"
#include <QWebView>
#include <QVBoxLayout>
#include <QDebug>
#include <QWebFrame>
#include <QThread>
#include "application.hpp"
#include <QMouseEvent>

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

  connect(webView, SIGNAL(loadStarted()),
          this,    SLOT(loadStarted()));


  connect(networkManager, SIGNAL(finished(QNetworkReply*)),
          this,           SLOT(singleRequestFinished(QNetworkReply*)));
}

SessionTab::~SessionTab()
{
  delete webView;
  delete webPage;
}

void SessionTab::visit(const QString& url)
{
  successfullLoad = false;
  webView->load(url);
}

bool SessionTab::waitForLoad(unsigned int t)
{
  unsigned long timeout = t;
  bool result;

  if (t == 0) {
    timeout = ULONG_MAX;
  }

  if (loadProgress != 100) {
    pageLoadedMutex.lock();
    qDebug() << "Wait for load";
    result = pageLoaded.wait(&pageLoadedMutex, timeout);
    qDebug() << "LOADED";
    pageLoadedMutex.unlock();
  } else {
    result = true;
  }

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

void SessionTab::loadStarted()
{
  successfullLoad = false;
  loadProgress = 0;
  _updateTabTitle();
  qDebug() << "Load started";
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

bool SessionTab::sendEvent(QEvent* e)
{
  QMouseEvent* me = static_cast<QMouseEvent*>(e);

  qDebug() << me->pos() << me->type() << me->button();

  webView->setEnabled(true);
  session->getApplication()->sendEvent(webView, e);
  webView->setEnabled(false);
}
