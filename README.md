MathWebSearch [![Build Status](https://secure.travis-ci.org/KWARC/mws.png?branch=master)](http://travis-ci.org/KWARC/mws)
=============

About
-----
The MathWebSearch system (MWS) is a content-based search engine for mathematical
formulae. It indexes MathML formulae, using a technique derived from automated
theorem proving: Substitution Tree Indexing. The software is licensed under the
GNU General Public License version 3.

Content
-------
* `config/` configuration files
* `data/` data used to run a MWS demo
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

The crawler executables require:
  - libhtmlcxx-dev
  - libicu-dev
  - libcurl4-gnutls-dev

The documentation target requires:
  - doxygen

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
[MWS Queries](https://trac.mathweb.org/MWS/wiki/MwsQuery) and returns
[MWS Answer Sets](https://trac.mathweb.org/MWS/wiki/MwsAnswset).

	bin/mwsd -I <harvest include dir> -p <port number>

For additional options, see:

    bin/mwsd --help
    bin/docs2harvest --help
    bin/mws-config help

To setup or remove `mwsd` as a global SysV service, use (as root):

    mws-config create -p 9090 -i data/zbl zbldemo
    mws-config enable zbldemo

This sets up MathWebSearch to serve the [ZBL demo harvests](data/zbl/) on
port 9090. To monitor, start or stop the service, use

	service mwsd_zbldemo [start|stop|status|...]

Output is logged to `/var/log/mwsd_zbldemo.log`. To serve different harvest
paths, create your own configurations and deploy the service.

Copyright
---------
The software in this project (binaries and sources) is released "as is",
under the GNU Public License version 3.
A copy of this license can be found in the root of this project,
under the file name LICENSE.

Credits
-------
Most of the code in the core repository was developed by Corneliu-Claudiu
Prodescu <c.prodescu@jacobs-university.de>, under the supervision of Prof.
Michael Kohlhase <m.kohlhase@jacobs-university.de>.
For a complete list of developers visit
https://trac.mathweb.org/MWS/wiki/DevTeam

Contact
-------
The easiest way to contact the developers is using the MathWebSearch
[mailing list](mailto:project-mathwebsearch-dev@lists.jacobs-university.de).

