#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>
#include <QList>

class QNetworkCookieJar;
class Application;
class QWidget;
class QVBoxLayout;
class QTabWidget;
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

  inline QTabWidget* getTabWidget() const {
    return tabWidget;
  }

  void createTab();

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  QWidget* window;
  QVBoxLayout* layout;
  QTabWidget* tabWidget;
  QList<SessionTab*> tabs;
};

#endif /* _SESSION_H_ */
