#include "json.hpp"

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

QString JSON::toJSON(const QString& value)
{
  QString s(value);
  return "\"" + s.replace("\"", "\\\"") + "\"";
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
  } else {
    return "null";
  }
}

QString JSON::response(const QString& code, const QString& additional)
{
  return QString("{ \"result\": \"%1\" %2 }").arg(code).arg(additional.isEmpty() ? "" : ", " + additional);
}
