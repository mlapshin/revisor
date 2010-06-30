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

  unsigned int createTab();

  inline unsigned int getTabsCount() const {
    return tabs.length();
  }

  SessionTab* getTab(unsigned int idx) const;

 private slots:
  void updateTabTitle(const QString& newTabTitle);

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  QWidget* window;
  QVBoxLayout* layout;
  QTabWidget* tabWidget;
  QList<SessionTab*> tabs;
};

#endif /* _SESSION_H_ */
