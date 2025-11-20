#include "base/logging.h"

namespace Ramulator
{

Logger_t Logging::create_logger(std::string, std::string)
{
    return std::make_shared<DummyLogger>();
}

Logger_t Logging::get(std::string)
{
    return std::make_shared<DummyLogger>();
}

bool Logging::_create_base_logger()
{
    return true;
}

} // namespace Ramulator