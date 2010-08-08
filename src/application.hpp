#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QApplication>
#include <QMap>
#include <QStringList>
#include <QScriptEngine>

class HttpServer;
class Dispatcher;
class Session;
class MainWindow;

class Application : public QApplication
{
  Q_OBJECT;
  typedef QMap<QString, Session*> SessionsMap;

 public:
  Application(int argc, char** argv);
  ~Application();

  Session* startSession(const QString& name);
  void stopSession(const QString& name);
  Session* getSession(const QString& name);
  inline int getSessionsCount() const {
    return sessions.size();
  }

  inline QScriptEngine* getScriptEngine() {
    return &scriptEngine;
  }

  inline MainWindow* getMainWindow() {
    return mainWindow;
  }

  int start();

 private:

  int argc;
  char** argv;
  bool initArgtable();
  int portNumber;
  QString listeningInterface;

  HttpServer* httpServer;
  Dispatcher* dispatcher;
  SessionsMap sessions;
  QScriptEngine scriptEngine;
  MainWindow* mainWindow;
};

#endif /* _APPLICATION_H_ */
