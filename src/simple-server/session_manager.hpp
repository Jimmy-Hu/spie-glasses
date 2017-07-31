#ifndef SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED
#define SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED

#include "session_fwd.hpp"
#include <set>
#include <api/pointer_compare.hpp>

using session_manager = std::set<session_ptr,api::uintptr_compare<session>>;
#endif // SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED
