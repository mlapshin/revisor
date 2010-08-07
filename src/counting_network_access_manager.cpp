#include "counting_network_access_manager.hpp"
#include <QDebug>
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QNetworkDiskCache>
#include <QNetworkReply>

CountingNetworkAccessManager::CountingNetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent)
{
  SessionTab* s = qobject_cast<SessionTab*>(parent);
  requestsCount = 0;

  connect(this, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(requestFinished(QNetworkReply*)));

  cache = new QNetworkDiskCache(this);
  cache->setCacheDirectory(QString("/tmp/revisor_cache/%1/%2/").arg(s->getSession()->getName()).arg(s->getName()));
  setCache(cache);
}

QNetworkReply* CountingNetworkAccessManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
  counterMutex.lock();
  requestsCount++;
  counterMutex.unlock();

  return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

void CountingNetworkAccessManager::requestFinished(QNetworkReply* reply)
{
  counterMutex.lock();
  requestsCount--;
  counterMutex.unlock();
}
