#ifndef API_SRC_SIMPLE_SERVER_IMAGE_SINK_HPP_INCLUDED
#define API_SRC_SIMPLE_SERVER_IMAGE_SINK_HPP_INCLUDED
#include <api/image_sink.hpp>
inline api::image_sink & image_sink() {
    static api::image_sink s;
    return s;
}
#endif // API_SRC_SIMPLE_SERVER_IMAGE_SINK_HPP_INCLUDED
