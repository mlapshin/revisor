#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>

class QNetworkCookieJar;
class Application;
class QWidget;

class Session : public QObject
{
  Q_OBJECT;

 public:
  Session(Application* a);
  ~Session();

  inline QNetworkCookieJar* getNetworkCookieJar() const {
    return cookieJar;
  }

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  QWidget* window;
};

#endif /* _SESSION_H_ */
