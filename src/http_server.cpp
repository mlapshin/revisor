#include "http_server.hpp"
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QtDebug>

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

  QMap<QString, QString> headers;
  QString requestMethod;
  QString path;
  QString requestBody;
  bool requestRead = false;
  int step = 0;
  int bodyLength = 0;

  while (requestRead == false) {
    if (step < 2) {
      if (socket->canReadLine()) {
        QString currentLine(socket->readLine(1024 * 1024));

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
            tokens[0].chop(1); // remove ':'
            tokens[1].chop(2); // remove newline

            headers.insert(tokens[0].toLower(), tokens[1]);
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
        if (!socket->waitForReadyRead(10 * 1000)) {
          requestRead = true;
        }
      }
    } else {
      // Read body
      if (requestBody.length() < bodyLength) {
        if (socket->bytesAvailable() > 0) {
          requestBody.append(socket->read(1024 * 1024));
        } else {
          if (!socket->waitForReadyRead(10 * 1000)) {
            requestRead = true;
          }
        }
      } else {
        requestRead = true;
      }
    }
  }

  if (requestMethod == "GET") {
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << "HTTP/1.0 200 Ok\r\n"
        "Content-Type: text/html; charset=\"utf-8\"\r\n"
        "\r\n"
        "<h1>OK</h1>\n";
  }

  socket->close();

  if (socket->state() == QTcpSocket::UnconnectedState) {
    delete socket;
  }

  qDebug() << headers << "\n------------------\n" << requestBody;

  commandReceived("Hello world!");
}

void HttpServer::discardClient()
{
  QTcpSocket* socket = (QTcpSocket*)sender();
  socket->deleteLater();
}
