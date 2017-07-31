#ifndef API_SRC_SIMPLE_SERVER_SESSION_FWD_HPP_INCLUDED
#define API_SRC_SIMPLE_SERVER_SESSION_FWD_HPP_INCLUDED

#include <memory>
class session;
using session_ptr = std::shared_ptr<session>;
using session_wptr = std::weak_ptr<session>;
using session_sptr = std::shared_ptr<session>;

#endif // API_SRC_SIMPLE_SERVER_SESSION_FWD_HPP_INCLUDED
