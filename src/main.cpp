#include <QtGui>
#include "http_server.hpp"

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);
  HttpServer srv(8080);

  return app.exec();
}
