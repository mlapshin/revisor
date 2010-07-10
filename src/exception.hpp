#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <stdexcept>

class Exception : public std::runtime_error
{
 public:
  Exception(const QString& message)
      : std::runtime_error(message.toStdString())
  {}

  inline QString getMessage() const {
    return what();
  }
};

#endif /* _EXCEPTION_H_ */
