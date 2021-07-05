if [ ! -d "/usr/include/opencv2" ]; then
    sudo mv /usr/include/opencv4/opencv2 /usr/include
fi

if [ ! -d "/usr/include/json" ]; then
    echo "Start building Jsoncpp library"
    git clone https://github.com/open-source-parsers/jsoncpp
    cd jsoncpp
    mkdir build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_CXX_COMPILER=clang++-11 ..
    make -j4
    sudo make install
    cd ../..
    echo "Build newest Jsoncpp library"
else
    echo "Jsoncpp Library has already built"
fi