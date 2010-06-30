#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <QScriptValue>
#include <QThread>

class Application;
class HttpServer;

class DeferredDispatcherResponseThread : public QThread
{
 public:
  const QScriptValue& getResponse() const {
    return response;
  }

 protected:
  QScriptValue response;
};

class DispatcherResponse
{
 public:
  bool deferred;
  QScriptValue response;
  DeferredDispatcherResponseThread* deferredThread;

  DispatcherResponse()
      : deferred(false), deferredThread(0) {}

  DispatcherResponse(const DispatcherResponse& r)
      : deferred(r.deferred), response(r.response), deferredThread(r.deferredThread) {}

  const DispatcherResponse& operator= (const DispatcherResponse& r) {
    deferred = r.deferred;
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
  Application* app;
};

#endif /* _DISPATCHER_H_ */
