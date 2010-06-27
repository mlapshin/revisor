#include "http_server.hpp"
#include "dispatcher.hpp"
#include "application.hpp"
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include <QtDebug>

#define BUFFER_SIZE (1024 * 1024)
#define TIMEOUT     (10 * 1000)

HttpServer::HttpServer(quint16 port, Application* a, Dispatcher* d)
    : QTcpServer(a), dispatcher(d), app(a)
{
  listen(QHostAddress::LocalHost, port);
}

void HttpServer::incomingConnection(int socket)
{
  QTcpSocket* s = new QTcpSocket(this);
  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
  s->setSocketDescriptor(socket);
}

QString HttpServer::handleCommand(const QStringMap& params)
{
  qDebug() << "Raw json: " << params["command"];
  dispatcher->dispatch(scriptEngine.evaluate("_foo = " + params["command"]));

  return QString("HTTP/1.0 200 OK\r\n"
                 "Content-Type: text/html; charset=\"utf-8\"\r\n"
                 "\r\n"
                 "<h1>OK</h1>\n");
}

void HttpServer::readClient()
{
  QTcpSocket* socket = (QTcpSocket*)sender();

  QStringMap headers;
  QStringMap requestParams;
  QString requestMethod;
  QString path;
  QString requestBody;
  bool requestRead = false;
  int step = 0;
  int bodyLength = 0;
  QString response;

  while (requestRead == false) {
    if (step < 2) {
      if (socket->canReadLine()) {
        QString currentLine(socket->readLine(BUFFER_SIZE));

        if (step == 0) {
          // GET /foo.html HTTP/1.1
          QStringList tokens = currentLine.split(" ");

          requestMethod = tokens[0];
          path = tokens[1];
          step = 1;
        } else if (step == 1) {
          // Headers
          if (currentLine != "\r\n") {
            QStringList tokens = currentLine.split(" ");
            if (tokens.length() == 2) {
              tokens[0].chop(1); // remove ':'
              tokens[1].chop(2); // remove newline

              headers.insert(tokens[0].toLower(), tokens[1]);
            }
          } else {
            // Last empty line before request body
            bool lengthPresent;
            step = 2;
            bodyLength = headers["content-length"].toInt(&lengthPresent);

            if (!lengthPresent) {
              bodyLength = 0;
            }
          }
        }
      } else {
        if (!socket->waitForReadyRead(TIMEOUT)) {
          requestRead = true;
        }
      }
    } else {
      // Read body
      if (requestBody.length() < bodyLength) {
        if (socket->bytesAvailable() > 0) {
          requestBody.append(socket->read(BUFFER_SIZE));
        } else {
          if (!socket->waitForReadyRead(TIMEOUT)) {
            requestRead = true;
          }
        }
      } else {
        requestRead = true;
      }
    }
  }

  if (requestBody.length() > 0) {
    QStringList paramPairs = requestBody.split("&");
    for(QStringList::iterator i = paramPairs.begin(); i != paramPairs.end(); i++) {
      QStringList pair = (*i).split("=");
      requestParams.insert(pair[0], QUrl::fromPercentEncoding(QByteArray(pair[1].replace('+', ' ').toAscii())));
    }
  }

  if (requestMethod == "POST" && path == "/command") {
    response = this->handleCommand(requestParams);
  }

  if (response.length() > 0) {
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << response;
  }

  socket->close();

  if (socket->state() == QTcpSocket::UnconnectedState) {
    delete socket;
  }
}

void HttpServer::discardClient()
{
  QTcpSocket* socket = (QTcpSocket*)sender();
  socket->deleteLater();
}
