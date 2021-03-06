#ifndef _SESSION_TAB_H_
#define _SESSION_TAB_H_

#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include <QMouseEvent>

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

  inline const QString& getFullName() const {
    return fullName;
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
  QVariant evaluateScript(const QString& script);
  void saveScreenshot(const QString& fileName, const QSize& viewportSize);
  bool sendEvent(QEvent* e);
  bool sendMouseEvent(QMouseEvent::Type type, const QPoint& point, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
  bool sendKeyEvent(QKeyEvent::Type type, Qt::Key key, Qt::KeyboardModifiers modifiers, const QString& text, bool autorep, ushort count);

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

  CountingNetworkAccessManager* networkManager;
  Session* session;
  QWebView* webView;
  WebPage* webPage;
  QString pageTitle;
  int loadProgress;
  QString name;
  QString fullName;
  bool successfullLoad;
};

#endif /* _SESSION_TAB_H_ */
