#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QApplication>

class HttpServer;

class Application : public QApplication
{
  Q_OBJECT;

 public:
  Application(int argc, char** argv);

 private slots:
  void commandReceived(const QString& cmd);

 private:
  HttpServer* httpServer;
};

#endif /* _APPLICATION_H_ */
