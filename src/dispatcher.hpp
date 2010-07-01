#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <QScriptValue>
#include <QThread>
#include "application.hpp"

class HttpServer;
class Dispatcher;

class DeferredDispatcherResponseThread : public QThread
{
 public:
  DeferredDispatcherResponseThread(Dispatcher* d);

  const QScriptValue& getResponse() const {
    return response;
  }

 protected:
  Dispatcher* dispatcher;
  QScriptValue response;
};

class DispatcherResponse
{
 public:
  bool deferred;
  Application* app;
  QScriptValue response;
  DeferredDispatcherResponseThread* deferredThread;

  DispatcherResponse(Application* a)
      : deferred(false), app(a), deferredThread(0) {}

  DispatcherResponse(const DispatcherResponse& r)
      : deferred(r.deferred), app(r.app), response(r.response), deferredThread(r.deferredThread) {}

  const DispatcherResponse& operator= (const DispatcherResponse& r) {
    deferred = r.deferred;
    response = r.response;
    deferredThread = r.deferredThread;
    app = r.app;

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
  Application* app;
};

#endif /* _DISPATCHER_H_ */
