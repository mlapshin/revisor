#include "json.hpp"
#include <QStringList>
#include <QDebug>

QString JSON::toJSON(bool value)
{
  return value ? "true" : "false";
}

QString JSON::toJSON(int value)
{
  return QString("%1").arg(value);
}

QString JSON::toJSON(unsigned int value)
{
  return QString("%1").arg(value);
}

QString JSON::toJSON(qulonglong value)
{
  return QString("%1").arg(value);
}

QString JSON::toJSON(qlonglong value)
{
  return QString("%1").arg(value);
}

QString JSON::toJSON(double f)
{
  return QString("%1").arg(f);
}

QString JSON::toJSON(const QDateTime& d)
{
  return toJSON(d.toString(Qt::ISODate));
}

QString JSON::toJSON(const QString& value)
{
  QString s(value);
  return "\"" + s.replace("\"", "\\\"") + "\"";
}

QString JSON::toJSON(const QVariantList& l)
{
  QString ret = "[";

  for (QVariantList::ConstIterator it = l.begin(); it != l.end(); it++) {
    ret += toJSON(*it);

    if ((it + 1) != l.end()) {
      ret += ", ";
    }
  }

  ret += "]";
  return ret;
}

QString JSON::toJSON(const QVariant& v)
{
  QVariant::Type t = v.type();

  if (t == QVariant::Invalid) {
    return "null";
  } else if (t == QVariant::Bool) {
    return toJSON(v.toBool());
  } else if (t == QVariant::Int) {
    return toJSON(v.toInt());
  } else if (t == QVariant::LongLong) {
    return toJSON(v.toLongLong());
  } else if (t == QVariant::Double) {
    return toJSON(v.toDouble());
  } else if (t == QVariant::String) {
    return toJSON(v.toString());
  } else if (t == QVariant::List) {
    return toJSON(v.toList());
  } else {
    return "null";
  }
}

QString JSON::response(const QString& code, const QString& additional)
{
  return QString("{ \"result\": \"%1\" %2 }").arg(code).arg(additional.isEmpty() ? "" : ", " + additional);
}

QString JSON::toJSON(const QList<QNetworkCookie>& cookies)
{
  QStringList components;

  for(QList<QNetworkCookie>::const_iterator it = cookies.begin(); it != cookies.end(); it++) {
    QString c;
    c += keyValue("name", QString((*it).name()));
    c += ", " + keyValue("value", QString((*it).value()));
    c += ", " + keyValue("domain", (*it).domain());
    c += ", " + keyValue("path", (*it).path());
    c += ", " + keyValue("http_only", (*it).isHttpOnly());
    c += ", " + keyValue("secure", (*it).isSecure());
    c += ", " + keyValue("expires_at", (*it).expirationDate());

    components << QString("{ %1 }").arg(c);
  }

  return QString("[ %1 ]").arg(components.join(", "));
}
