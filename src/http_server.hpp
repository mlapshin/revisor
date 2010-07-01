#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <QTcpServer>
#include <QScriptValue>
#include "dispatcher.hpp"

class Application;
class QSignalMapper;

class HttpServer : public QTcpServer
{
  typedef QMap<QString, QString> QStringMap;
  Q_OBJECT;

 public:
  HttpServer(quint16 port, Application* app, Dispatcher* d);
  void incomingConnection(int socket);

 private slots:
  void readClient();
  void discardClient();
  void deferredResponseReady(QObject* socket);

 private:
  void handleCommand(const QStringMap& params, QTcpSocket* socket);
  void sendRawResponse(const QString& response, QTcpSocket* socket);

  Dispatcher* dispatcher;
  Application* app;
  QSignalMapper* signalMapper;
};

#endif /* _HTTP_SERVER_H_ */
