#include "counting_network_access_manager.hpp"
#include <QDebug>

CountingNetworkAccessManager::CountingNetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent)
{
  requestsCount = 0;
  connect(this, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(requestFinished(QNetworkReply*)));
}

QNetworkReply* CountingNetworkAccessManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
  counterMutex.lock();
  requestsCount++;
  counterMutex.unlock();

  return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

void CountingNetworkAccessManager::requestFinished(QNetworkReply* r)
{
  counterMutex.lock();
  requestsCount--;
  counterMutex.unlock();
}
