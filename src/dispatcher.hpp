#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <QScriptValue>
#include <QThread>
#include "application.hpp"

class HttpServer;
class Dispatcher;

class DeferredDispatcherResponseThread : public QThread
{
  Q_OBJECT;

 public:
  DeferredDispatcherResponseThread(Dispatcher* d);

  const QString& getResponse() const {
    return response;
  }

 protected:
  Dispatcher* dispatcher;
  QString response;
};

class DispatcherResponse
{
 public:
  QString response;
  DeferredDispatcherResponseThread* deferredThread;

  DispatcherResponse()
      : deferredThread(0) {}

  DispatcherResponse(const DispatcherResponse& r)
      : response(r.response), deferredThread(r.deferredThread) {}

  const DispatcherResponse& operator= (const DispatcherResponse& r) {
    response = r.response;
    deferredThread = r.deferredThread;

    return *this;
  }
};

class Dispatcher: public QObject
{
  Q_OBJECT;

 public:
  Dispatcher(Application* app);
  DispatcherResponse dispatch(const QScriptValue& command);

 private:

  DispatcherResponse handleSessionCommand(const QString& commandName, const QScriptValue& cmd);
  DispatcherResponse handleSessionTabCommand(const QString& commandName, const QScriptValue& cmd);

  Application* app;
};

#endif /* _DISPATCHER_H_ */
