#ifndef _SESSION_TAB_H_
#define _SESSION_TAB_H_

#include <QObject>

class Session;
class CountingNetworkAccessManager;
class QWebView;

class SessionTab : public QObject
{
  Q_OBJECT;
 public:
  SessionTab(Session* s);
  ~SessionTab();

  inline CountingNetworkAccessManager* getNetworkAccessManager() const {
    return networkManager;
  }

  inline QWebView* getWebView() const {
    return webView;
  }

  void visit(const QString& url);

 public slots:
  void updateTitle(const QString& t);
  void updateProgress(int p);
  void loadFinished(bool s);

 signals:
  void titleChanged(const QString& newTitle);

 private:
  void _updateTabTitle();

  CountingNetworkAccessManager* networkManager;
  Session* session;
  QWebView* webView;
  QString pageTitle;
  int loadProgress;
};

#endif /* _SESSION_TAB_H_ */
