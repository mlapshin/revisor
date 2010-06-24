#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <QTcpServer>

class HttpServer : public QTcpServer
{
  Q_OBJECT;

  public:
  HttpServer(quint16 port, QObject* parent = 0);

  void incomingConnection(int socket);

  void pause();
  void resume();

  signals:
  void commandReceived(const QString& command);

  private slots:
  void readClient();
  void discardClient();

  private:
  bool disabled;
};

#endif /* _HTTP_SERVER_H_ */
