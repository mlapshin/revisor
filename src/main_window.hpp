#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QWidget>
#include <QMap>

class QTabWidget;
class SessionTab;
class Application;

class MainWindow : public QWidget
{
  Q_OBJECT;
  typedef QMap<QString, SessionTab*> SessionTabsMap;

 public:
  MainWindow(Application* a);
  void closeEvent(QCloseEvent* event);

  void registerSessionTab(SessionTab* tab);
  void unregisterSessionTab(SessionTab* tab);
  void raiseSessionTab(SessionTab* tab);

 public slots:
  void updateTabTitle(const QString& newTitle);

 private:
  void updateVisibility();

  Application* app;
  QTabWidget* tabWidget;
  SessionTabsMap tabs;
};

#endif /* _MAIN_WINDOW_H_ */
