#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <QTcpServer>
#include <QScriptValue>
#include <QScriptEngine>

class Dispatcher;
class Application;

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

 private:

  QString handleCommand(const QStringMap& params);

  Dispatcher* dispatcher;
  Application* app;
  QScriptEngine scriptEngine;
};

#endif /* _HTTP_SERVER_H_ */
