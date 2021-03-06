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
#include <QKeyEvent>

class SleepyThread : public QThread
{
 public:
  static void msleep(unsigned long msec) {
    QThread::msleep(msec);
  }
};

SessionTab::SessionTab(Session* s, const QString& n)
    : QObject(s), session(s), loadProgress(0), name(n), fullName(QString("%1/%2").arg(session->getName()).arg(name))
{
  successfullLoad = false;
  networkManager = new CountingNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());

  webPage = new WebPage(this);
  webPage->setNetworkAccessManager(networkManager);

  webView = new QWebView();
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
    qDebug() << "Page loaded";
    pageLoadedMutex.unlock();
  } else {
    qDebug() << "Page already loaded";
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
  QVariant ret = webView->page()->mainFrame()->evaluateJavaScript(script);
  return ret;
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

  title += QString(" [%1]").arg(fullName);
  emit titleChanged(title);
}

void SessionTab::singleRequestFinished(QNetworkReply* reply)
{
}

void SessionTab::saveScreenshot(const QString& fileName, const QSize& viewportSize)
{
  webPage->saveScreenshot(fileName, viewportSize);
}

bool SessionTab::sendEvent(QEvent* e)
{
  webView->setEnabled(true);
  bool ret = session->getApplication()->sendEvent(webView, e);
  webView->setEnabled(false);

  return ret;
}

bool SessionTab::sendKeyEvent(QKeyEvent::Type type, Qt::Key key, Qt::KeyboardModifiers modifiers, const QString& text, bool autorep, ushort count)
{
  QKeyEvent event(type, key, modifiers, text, autorep, count);
  bool ret = sendEvent(&event);

  return ret;
}

bool SessionTab::sendMouseEvent(QMouseEvent::Type type, const QPoint& point, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
  QWebFrame* frame = webPage->mainFrame();
  QPoint relativePoint;

  unsigned int scrollX = 0;
  unsigned int scrollY = 0;

  QPoint savedScrollPos = frame->scrollPosition();

  unsigned int w = frame->geometry().width() - frame->scrollBarGeometry(Qt::Vertical).width();
  unsigned int h = frame->geometry().height() - frame->scrollBarGeometry(Qt::Horizontal).height();

  // unsigned int maxScrollX = frame->scrollBarMaximum(Qt::Horizontal);
  // unsigned int maxScrollY = frame->scrollBarMaximum(Qt::Vertical);

  if (point.x() > (w - 1)) {
    scrollX = point.x() - w + 1;
    relativePoint.setX(w - 1);
  } else {
    relativePoint.setX(point.x());
  }

  if (point.y() > (h - 1)) {
    scrollY = point.y() - h + 1;
    relativePoint.setY(h - 1);
  } else {
    relativePoint.setY(point.y());
  }

  QMouseEvent event(type, relativePoint, button, buttons, modifiers);
  frame->setScrollPosition(QPoint(scrollX, scrollY));

  bool ret = sendEvent(&event);

  qDebug() << "Mouse event" << type << relativePoint;
  frame->setScrollPosition(savedScrollPos);

  return ret;
}
