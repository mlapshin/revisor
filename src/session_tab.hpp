#ifndef _SESSION_TAB_H_
#define _SESSION_TAB_H_

#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>

class Session;
class CountingNetworkAccessManager;
class QWebView;
class QNetworkReply;

class SessionTab : public QObject
{
  Q_OBJECT;
 public:
  SessionTab(Session* s, const QString& name);
  ~SessionTab();

  inline CountingNetworkAccessManager* getNetworkAccessManager() const {
    return networkManager;
  }

  inline QWebView* getWebView() const {
    return webView;
  }

  inline const QString& getName() const {
    return name;
  }

  void visit(const QString& url);
  bool waitForLoad(unsigned int timeout = 0);
  QVariant evaluateScript(const QString& script);

 public slots:
  void updateTitle(const QString& t);
  void updateProgress(int p);
  void loadFinished(bool s);
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
  QString pageTitle;
  int loadProgress;
  QString name;
};

#endif /* _SESSION_TAB_H_ */
