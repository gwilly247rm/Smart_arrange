#include "WiseModbusException.h"

WiseModbusException::WiseModbusException(const std::string& what)
    : std::runtime_error(what) {}
