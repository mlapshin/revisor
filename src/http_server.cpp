#include "http_server.hpp"
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>

HttpServer::HttpServer(quint16 port, QObject* parent)
    : QTcpServer(parent), disabled(false)
{
  listen(QHostAddress::LocalHost, port);
}


void HttpServer::incomingConnection(int socket)
{
  if (disabled)
    return;

  QTcpSocket* s = new QTcpSocket(this);
  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
  s->setSocketDescriptor(socket);
}

void HttpServer::pause()
{
  disabled = true;
}

void HttpServer::resume()
{
  disabled = false;
}

void HttpServer::readClient()
{
  if (disabled) {
    return;
  }

  QTcpSocket* socket = (QTcpSocket*)sender();
  if (socket->canReadLine()) {
    QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));

    if (tokens[0] == "GET") {
      QTextStream os(socket);
      os.setAutoDetectUnicode(true);
      os << "HTTP/1.0 200 Ok\r\n"
          "Content-Type: text/html; charset=\"utf-8\"\r\n"
          "\r\n"
          "<h1>Nothing to see here</h1>\n"
         << QDateTime::currentDateTime().toString() << "\n";
      socket->close();

      if (socket->state() == QTcpSocket::UnconnectedState) {
        delete socket;
      }

      commandReceived("Hello world!");
    }
  }
}

void HttpServer::discardClient()
{
  QTcpSocket* socket = (QTcpSocket*)sender();
  socket->deleteLater();
}
