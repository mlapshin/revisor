#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <QTcpServer>
#include <QScriptValue>
#include "dispatcher.hpp"

class Application;

class HttpServer : public QTcpServer
{
  typedef QMap<QString, QString> QStringMap;
  Q_OBJECT;

 public:
  HttpServer(const QString& host, quint16 port, Application* app, Dispatcher* d);
  void incomingConnection(int socket);

 private slots:
  void readClient();
  void discardClient();
  void deferredResponseReady();

 private:
  void handleCommand(const QStringMap& params, QTcpSocket* socket);
  void sendRawResponse(const QString& response, QTcpSocket* socket);

  Dispatcher* dispatcher;
  Application* app;
  QMap<DeferredDispatcherResponseThread*, QTcpSocket*> deferredSocketsMap;
};

#endif /* _HTTP_SERVER_H_ */
