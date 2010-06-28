#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <QScriptValue>

class Application;
class HttpServer;

class Dispatcher: public QObject
{
  Q_OBJECT;

 public:
  Dispatcher(Application* app);
  void dispatch(const QScriptValue& command);

 private:
  Application* app;
};

#endif /* _DISPATCHER_H_ */
