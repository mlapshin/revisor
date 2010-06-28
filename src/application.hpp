#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QApplication>
#include <QList>

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

 private:
  HttpServer* httpServer;
  Dispatcher* dispatcher;
  QList<Session*> sessions;
};

#endif /* _APPLICATION_H_ */
