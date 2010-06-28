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

 private:
  QNetworkAccessManager* networkManager;
  Session* session;
  QWebView* webView;
};

#endif /* _SESSION_TAB_H_ */
