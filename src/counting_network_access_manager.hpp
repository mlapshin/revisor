#ifndef _COUNTING_NETWORK_ACCESS_MANAGER_H_
#define _COUNTING_NETWORK_ACCESS_MANAGER_H_

#include <QNetworkAccessManager>
#include <QMutex>

class QNetworkDiskCache;

class CountingNetworkAccessManager : public QNetworkAccessManager
{
  Q_OBJECT;

 public:
  CountingNetworkAccessManager(QObject* parent = 0);

  inline unsigned int getRequestsCount() const {
    return requestsCount;
  }

 private slots:
  void requestFinished(QNetworkReply* reply);

 protected:
  virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

 private:
  QMutex counterMutex;
  unsigned int requestsCount;
  QNetworkDiskCache* cache;
};

#endif /* _COUNTING_NETWORK_ACCESS_MANAGER_H_ */
