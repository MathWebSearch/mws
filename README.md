MathWebSearch [![Build Status](https://github.com/MathWebSearch/mws/workflows/CI/badge.svg)](https://github.com/MathWebSearch/mws/actions)
=============

About
-----
The MathWebSearch system (MWS) is a content-based search engine for mathematical formulae. 
It indexes MathML formulae, using a technique derived from automated theorem proving: Substitution Tree Indexing. 
The software is licensed under the GNU General Public License version 3.

Demos
------
can be found at http://search.mathweb.org, in particular, the arXiv demo is at http://arxivsearch.mathweb.org

Content
-------
* `analytics/` user defined analytics source files
* `config/` configuration files
* `data/` data used to run a MWS demo
* `doc/` documentation for MWS users
* `scripts/` utility scripts
* `src/` source code
* `test/` test source code and data
* `third_party/` third party source code
* `CMakeLists.txt` CMake build script
* `LICENSE` copy of the license under which this software is distributed
* `Makefile` build makefile
* `README.md` documentation overview about the project
* `TODO` project TODOs which have not materialized into tickets

Compiling and Testing
---------------------
Compiling of the source tree is automated via CMake. You can build the sources
using the following command:

	make

Binaries are built in the `bin/` directory, while documentation in `bin/docs`.
To select or de-select which components to compile, use:

	make config

To run the tests, use:

	make test

Finally, install the binaries using:

    make install


Dependencies
------------
To build this software, one needs:
  - g++ (with pthread) (>= 4.4)
  - cmake              (>= 2.6)
  - make
  - pkg-config

The core MathWebSearch executables require:
  - libmicrohttpd      (>= 0.4)
  - libxml2
  - libleveldb
  - libsnappy
  - libjson-c
  - libjson0-dev

The crawler executables require:
  - libhtmlcxx-dev
  - libicu-dev
  - libcurl4-gnutls-dev

The documentation target requires:
  - doxygen

The test target requires:
  - netcat
  - curl

The config step requires:
  - cmake-curses-gui

To install all build, runtime and test dependencies on a sufficiently new Debian / Ubuntu machine you can use:

```bash
apt-get install g++ cmake make pkg-config libmicrohttpd12 libxml2 libleveldb1v5 \
  libsnappy1v5 libjson-c3 libhtmlcxx3v5 libgnutlsxx28 libicu57 libcurl3-gnutls
```

To install all build, runtime and test dependencies on Mac OS, you can use [homebrew](https://brew.sh):

```bash
brew install gcc make cmake pkg-config libmicrohttpd libxml2 leveldb snappy json-c \
  htmlcxx icu4c gnutls netcat curl

```

Makefile targets
----------------
* `all` builds all the binaries of the project and tests
* `clean` cleans the build
* `config` brings up the cmake CLI configuration tool
* `doc` generates the documentation of the project
* `test` runs project tests
* `help` display the complete list of targets
* `install` installs `mwsd`, `docs2harvest`, `mws-config` on your system

Usage
-----
To use the Crawler, one needs to start the crawler with the defined website, 
the count of sites to crawl and optionaly: the start of the crawling and links
to be skipped when crawling.

Another way to generate harvests is via `docs2harvest`. This takes as
arguments XHTML documents and crawls them, creating harvests files. To
crawl a repository of XHTML documents, use:

    find . -name *.xhtml | xargs -n 10 bin/docs2harvest -o /path/to/harvests

The executable `mwsd` starts the main MWS server. This takes as argument a
harvest include path which is used to load document data, and a port which
where the data is served via HTTP. It accepts HTTP POST requests with
[MWS Queries](https://github.com/KWARC/mws/wiki/MWSQuery) and returns
[MWS Answer Sets](https://github.com/KWARC/mws/wiki/MWSAnswerSet).

	bin/mwsd -I <harvest include dir> -p <port number>

For additional options, see:

    bin/mwsd --help
    bin/docs2harvest --help
    bin/mws-config help

To setup or remove `mwsd` as a global SysV service, use (as root):

    mws-config create -p 9090 -i data/zbl zbldemo
    mws-config enable zbldemo

This will deploy MathWebSearch to serve the
[ZBL demo harvests](data/zbl/) on port 9090. To monitor, start or stop
the service, use

	service mwsd_zbldemo [start|stop|status|...]

Output is logged to `/var/log/mwsd_zbldemo.log`. To serve different harvest
paths, create your own configurations and deploy the service.

<!--
Direct install (no longer maintained)
--------------
You can install MWS directly from this
[PPA](https://launchpad.net/~radu-hambasan/+archive/ubuntu/math-web-search):

    sudo add-apt-repository ppa:radu-hambasan/math-web-search
    sudo apt-get install mws
-->

Usage with Docker
-----------------

This repository contains a Dockerfile for using the MWS Daemon. 
It can be found as the [mathwebsearch/mathwebsearch](https://hub.docker.com/r/mathwebsearch/mathwebsearch/) automated build on Docker Hub and used as follows:

  docker run -v /path/to/harvests:/data/ -p 8080:8080 mathwebsearch/mathwebsearch

The image is configured to serve harvests from a `/data/` volume on port 8080. 

Copyright
---------
The software in this project (binaries and sources) is released "as is",
under the GNU Public License version 3.
A copy of this license can be found in the root of this project,
under the file name LICENSE.

Credits
-------
Most of the code in the core repository was developed by [Corneliu-Claudiu Prodescu](https://kwarc.info/people/cprodescu/), under the supervision of Prof.
[Michael Kohlhase](https://kwarc.info/people/mkohlhase/).
For a complete list of developers visit
https://github.com/KWARC/mws/graphs/contributors

Contact
-------
The easiest way to contact the developers is using the MathWebSearch
[mailing list](mailto:project-mathwebsearch-dev@lists.jacobs-university.de).

