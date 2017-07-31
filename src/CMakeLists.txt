###
#
# global:
#
find_package (
  Boost
  COMPONENTS
  system
  # filesystem
  # regex
  # chrono
  # thread
  #context
  )
###
#
# target: chat_server
#
set(CHAT_SERVER_SRCS
  "chat_server.cpp"
  "chat_message.hpp"
)
add_executable(chat_server ${CHAT_SERVER_SRCS})
set_target_properties(
  chat_server
  PROPERTIES
  OUTPUT_NAME "chat_server"
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
  C_STANDARD              90
  C_STANDARD_REQUIRED     ON
  CXX_STANDARD            14
  CXX_STANDARD_REQUIRED   ON
  )
target_include_directories (
  chat_server
  PRIVATE
  ${Boost_INCLUDE_DIRS}
  )
target_link_libraries (
  chat_server
  ${Boost_LIBRARIES}
  )
if(WIN32)
  target_link_libraries(chat_server wsock32 ws2_32) # as boost.asio dependency
endif()

###
#
# chat_client
#
set(CHAT_CLIENT_SRCS
  "chat_client.cpp"
  "chat_message.hpp"
)
add_executable(chat_client ${CHAT_CLIENT_SRCS})
set_target_properties(
  chat_client
  PROPERTIES
  OUTPUT_NAME "chat_client"
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
  C_STANDARD              90
  C_STANDARD_REQUIRED     ON
  CXX_STANDARD            14
  CXX_STANDARD_REQUIRED   ON
  )
target_include_directories (
  chat_client
  PRIVATE
  ${Boost_INCLUDE_DIRS}
  )
target_link_libraries (
  chat_client
  ${Boost_LIBRARIES}
  )
if(WIN32)
  target_link_libraries(chat_client wsock32 ws2_32) # as boost.asio dependency
endif()
