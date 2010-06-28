#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>

class QNetworkCookieJar;
class QNetworkAccessManager;
class Application;
class QWidget;
class QWebView;

class Session : public QObject
{
  Q_OBJECT;

 public:
  Session(Application* a);
  ~Session();

  inline QNetworkAccessManager* getNetworkAccessManager() const {
    return networkManager;
  }

 private:
  QNetworkCookieJar* cookieJar;
  QNetworkAccessManager* networkManager;
  Application* app;
  QWidget* window;
  QWebView* webView;
};

#endif /* _SESSION_H_ */
