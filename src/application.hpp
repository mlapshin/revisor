#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QApplication>
#include <QList>
#include <QStringList>
#include <QScriptEngine>
#include <argtable2.h>

class HttpServer;
class Dispatcher;
class Session;

class Application : public QApplication
{
  Q_OBJECT;

 public:
  Application(int argc, char** argv);
  ~Application();

  int startSession();
  void stopSession(int index);
  Session* getSession(int i);
  inline int getSessionsCount() const {
    return sessions.length();
  }

  QScriptEngine* getScriptEngine() {
    return &scriptEngine;
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
  QList<Session*> sessions;
  QScriptEngine scriptEngine;
};

#endif /* _APPLICATION_H_ */
