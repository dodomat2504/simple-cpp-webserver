# Simple-Cpp-webserver

This project is a simple webserver written in C++. You can use it to create API endpoints. Just download the source code and integrate it into your project. You can inherit from the HTTPServer to add the functionality to your class. The only dependency is the [jsoncpp library](https://github.com/open-source-parsers/jsoncpp).

I installed the jsoncpp library with the following commands:
```bash
git clone https://github.com/open-source-parsers/jsoncpp.git

cd jsoncpp && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=release -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" .. && make && sudo make install
```

___

This project will only work on linux systems due to the used headers.
