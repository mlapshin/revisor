#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QApplication>

class HttpServer;
class Dispatcher;

class Application : public QApplication
{
  Q_OBJECT;

 public:
  Application(int argc, char** argv);

 private:
  HttpServer* httpServer;
  Dispatcher* dispatcher;
};

#endif /* _APPLICATION_H_ */
