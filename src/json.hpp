#ifndef _JSON_H_
#define _JSON_H_

#include <QString>
#include <QVariant>

class JSON
{
 public:
  template<typename T> static QString keyValue(const QString& key, const T& value)
  {
    return "\"" + key + "\": " + JSON::toJSON(value);
  }

  static QString response(const QString& code, const QString& additional = "");

 private:
  static QString toJSON(bool value);
  static QString toJSON(double value);
  static QString toJSON(int value);
  static QString toJSON(qlonglong value);
  static QString toJSON(unsigned int value);
  static QString toJSON(qulonglong value);

  static QString toJSON(const QVariant& v);
  static QString toJSON(const QString& value);
};

#endif /* _JSON_H_ */
