#!/bin/sh
g++ \
    "${@}" \
    -I${MSYSTEM_PREFIX}/local/include \
    -I./include \
    -L${MSYSTEM_PREFIX}/local/lib \
    -lfmt \
    -lz \
    -lmsgpackc \
    -lwsock32 \
    -lws2_32 \
    -llz4 \
    -lboost_system-mt \
    -lboost_thread-mt \
    -lboost_filesystem-mt \
    -lopencv_calib3d \
    -lopencv_core \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_shape \
    -lopencv_stitching \
    -lopencv_superres \
    -lopencv_video \
    -lopencv_videoio \
    -lopencv_videostab \
    -lopencv_viz \
    -o$(echo "$1" | sed 's/\.cpp$//g')

    # -lyaml-cpp \
