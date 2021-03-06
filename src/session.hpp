#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>
#include <QMap>
#include <QWidget>
#include <QList>
#include <QNetworkCookie>
#include <QUrl>

class QNetworkCookieJar;
class Application;
class QVBoxLayout;
class QTabWidget;
class SessionTab;
class Session;

class Session : public QObject
{
  Q_OBJECT;
  typedef QMap<QString, SessionTab*> SessionTabsMap;

 public:
  Session(Application* a, const QString& name);
  virtual ~Session();

  inline QNetworkCookieJar* getNetworkCookieJar() const {
    return cookieJar;
  }

  inline const QString& getName() const {
    return name;
  }

  inline Application* getApplication() const {
    return app;
  }

  SessionTab* createTab(const QString& tabName);
  void destroyTab(const QString& name);

  inline int getTabsCount() const {
    return tabs.size();
  }

  SessionTab* getTab(const QString& tabName) const;

  void setCookiesFor(const QList<QNetworkCookie>& cookies, const QUrl& forUrl);
  QList<QNetworkCookie> getCookiesFor(const QUrl& forUrl) const;

 private:
  int getTabIndex(const QString& tabName);

  QNetworkCookieJar* cookieJar;
  Application* app;
  SessionTabsMap tabs;
  QString name;
};

#endif /* _SESSION_H_ */
