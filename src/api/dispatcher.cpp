
#include <api/dispatcher.hpp>

namespace api {
    boost::system::error_code dispatcher::m_protocol_error = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
}
