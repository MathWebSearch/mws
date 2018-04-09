FROM ubuntu:xenial

# Install dependencies
# These come first to take advantage of the build cache
RUN apt-get update \
    && apt-get -y install \
        cmake \
        g++ \
        make \
        pkg-config \
        libmicrohttpd-dev \
        libxml2-dev \
        libleveldb-dev \
        libsnappy-dev \
        libjson0 \
        libjson0-dev \
        libhtmlcxx3v5 \
        libhtmlcxx-dev \
        libgnutls-dev \
        libicu-dev \
        libcurl4-gnutls-dev \
        doxygen \
        netcat \
        curl \
    && apt-get clean

# Setup file structure under /mws/
ADD analytics/ /mws/analytics
ADD config/ mws/config
ADD doc/ /mws/doc
ADD scripts/ mws/scripts
ADD src /mws/src
ADD test/ mws/test
ADD third_party/ mws/third_party/

ADD .arcconfig /mws/
ADD .arclint /mws/
ADD .clang-format /mws/
ADD CMakeLists.txt /mws/
ADD LICENSE /mws/
ADD Makefile /mws/

# Run a build
WORKDIR /mws/
RUN make all