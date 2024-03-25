cd fltk
autogen.sh
CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ CPP=i686-w64-mingw32-cpp ./configure --enable-shared
make
make install