#ifndef _SESSION_TAB_H_
#define _SESSION_TAB_H_

#include <QObject>

class Session;
class QNetworkAccessManager;
class QWebView;

class SessionTab : public QObject
{
  Q_OBJECT;
 public:
  SessionTab(Session* s);
  ~SessionTab();

  inline QNetworkAccessManager* getNetworkAccessManager() const {
    return networkManager;
  }

  inline QWebView* getWebView() const {
    return webView;
  }

 public slots:
  void updateTitle(const QString& t);
  void updateProgress(int p);

 signals:
  void titleChanged(const QString& newTitle);

 private:
  void _updateTabTitle();

  QNetworkAccessManager* networkManager;
  Session* session;
  QWebView* webView;
  QString pageTitle;
  int loadProgress;
};

#endif /* _SESSION_TAB_H_ */
