#ifndef _SESSION_TAB_H_
#define _SESSION_TAB_H_

#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>

class Session;
class CountingNetworkAccessManager;
class QWebView;
class WebPage;
class QNetworkReply;

class SessionTab : public QObject
{
  Q_OBJECT;

 public:
  SessionTab(Session* s, const QString& name);
  virtual ~SessionTab();

  inline CountingNetworkAccessManager* getNetworkAccessManager() const {
    return networkManager;
  }

  inline QWebView* getWebView() const {
    return webView;
  }

  inline const QString& getName() const {
    return name;
  }

  inline bool isLoadSuccessfull() const {
    return successfullLoad;
  }

  inline Session* getSession() const {
    return session;
  }

  void setConfirmAnswer(bool a);
  void setPromptAnswer(const QString& a, bool canceled);

  void visit(const QString& url);
  bool waitForLoad(unsigned int timeout = 0);
  void waitForAllRequestsFinished(unsigned int waitBefore = 0, unsigned int waitAfter = 0, unsigned int waitTimeout = 0);
  bool waitForTrueEvaluation(const QString& script, unsigned int retryInterval, unsigned int tries);
  QVariant evaluateScript(const QString& script);
  void saveScreenshot(const QString& fileName, const QSize& viewportSize);
  bool sendEvent(QEvent* e);

 public slots:
  void updateTitle(const QString& t);
  void updateProgress(int p);
  void loadFinished(bool s);
  void loadStarted();
  void singleRequestFinished(QNetworkReply* reply);

 signals:
  void titleChanged(const QString& newTitle);

 private:
  void _updateTabTitle();

  QWaitCondition pageLoaded;
  QMutex pageLoadedMutex;

  QWaitCondition requestsFinished;
  QMutex requestsFinishedMutex;

  CountingNetworkAccessManager* networkManager;
  Session* session;
  QWebView* webView;
  WebPage* webPage;
  QString pageTitle;
  int loadProgress;
  QString name;
  bool successfullLoad;
};

#endif /* _SESSION_TAB_H_ */
