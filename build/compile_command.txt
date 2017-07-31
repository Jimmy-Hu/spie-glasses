# delete old files(if files are exist) in Windows
echo -e "delete old files"
test -f ./cmake_install.cmake && rm -rf ./cmake_install.cmake && pwd && echo -e "/cmake_install.cmake is removed"
test -f ./CMakeCache.txt && rm -rf ./CMakeCache.txt && pwd  && echo -e "/CMakeCache.txt is removed"
test -d ./CMakeFiles && rm -rf ./CMakeFiles && pwd  && echo -e "/CMakeFiles folder is removed"
test -f ./Makefile && rm -rf ./Makefile && pwd  && echo -e "/Makefile is removed"
test -d ./dlib_build && rm -rf ./dlib_build && pwd  && echo -e "/dlib_build folder is removed"
test -d ./export && rm -rf ./export && pwd  && echo -e "/export folder is removed"
test -f ./.ninja_deps && rm -rf ./.ninja_deps && pwd  && echo -e "/.ninja_deps is removed"
test -f ./.ninja_log && rm -rf ./.ninja_log && pwd  && echo -e "/.ninja_log is removed"
test -f ./build.ninja && rm -rf ./build.ninja && pwd  && echo -e "/build.ninja is removed"
test -f ./rules.ninja && rm -rf ./rules.ninja && pwd  && echo -e "/rules.ninja is removed"
# config and build main.cpp in ../src folder
echo -e "config and build main.cpp in ../src folder"
cmake -G "Ninja" -DCMAKE_PREFIX_PATH=/mingw64/local -DCMAKE_INSTALL_PREFIX=./install ..
cmake --build .
# run the executable file(windows)
echo -e "run the executable file"
test -f ./export/bin/server.exe && ./export/bin/server.exe 5000
#test -f ./export/bin/image-client.exe && ./export/bin/image-client.exe 127.0.0.1 5000
#test -f ./export/bin/audio-client.exe && ./export/bin/audio-client.exe 127.0.0.1 5000


