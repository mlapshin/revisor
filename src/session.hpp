#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>

class QNetworkCookieJar;
class Application;
class QWidget;
class SessionTab;

class Session : public QObject
{
  Q_OBJECT;

 public:
  Session(Application* a);
  ~Session();

  inline QNetworkCookieJar* getNetworkCookieJar() const {
    return cookieJar;
  }

  inline QWidget* getWindow() const {
    return window;
  }

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  QWidget* window;
  SessionTab* defaultTab;
};

#endif /* _SESSION_H_ */
