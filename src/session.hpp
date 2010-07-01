#ifndef _SESSION_H_
#define _SESSION_H_

#include <QObject>
#include <QList>
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

 public:
  Session(Application* a);
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

  unsigned int createTab();

  inline unsigned int getTabsCount() const {
    return tabs.length();
  }

  SessionTab* getTab(int idx) const;

 private slots:
  void updateTabTitle(const QString& newTabTitle);

 private:
  QNetworkCookieJar* cookieJar;
  Application* app;
  SessionWindow* window;
  QVBoxLayout* layout;
  QTabWidget* tabWidget;
  QList<SessionTab*> tabs;
};

#endif /* _SESSION_H_ */
