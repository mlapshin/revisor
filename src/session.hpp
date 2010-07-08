#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>
#include <QMap>
#include <QWidget>

class QNetworkCookieJar;
class Application;
class QVBoxLayout;
class QTabWidget;
class SessionTab;
class SessionWindow;

class SessionWindow : public QWidget
{
  Q_OBJECT;

 public:
  SessionWindow();
  void closeEvent(QCloseEvent* event);
};

class Session : public QObject
{
  Q_OBJECT;
  typedef QMap<QString, SessionTab*> SessionTabsMap;

 public:
  Session(Application* a, const QString& name);
  ~Session();

  inline QNetworkCookieJar* getNetworkCookieJar() const {
    return cookieJar;
  }

  inline SessionWindow* getWindow() const {
    return window;
  }

  inline QTabWidget* getTabWidget() const {
    return tabWidget;
  }

  inline const QString& getName() const {
    return name;
  }

  SessionTab* createTab(const QString& tabName);

  inline int getTabsCount() const {
    return tabs.size();
  }

  SessionTab* getTab(const QString& tabName) const;

 private slots:
  void updateTabTitle(const QString& newTabTitle);

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  SessionWindow* window;
  QVBoxLayout* layout;
  QTabWidget* tabWidget;
  SessionTabsMap tabs;
  QString name;
};

#endif /* _SESSION_H_ */
