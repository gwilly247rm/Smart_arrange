#ifndef __WISEMODBUSEXCEPTION_H__
#define __WISEMODBUSEXCEPTION_H__

#include <stdexcept>

class WiseModbusException : public std::runtime_error {
   public:
    WiseModbusException(const std::string& what);
};

#endif
