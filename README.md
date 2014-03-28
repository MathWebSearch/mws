MathWebSearch [![Build Status](https://secure.travis-ci.org/KWARC/mws.png?branch=master)](http://travis-ci.org/KWARC/mws)
=============

About
-----
The MathWebSearch system (MWS) is a content-based search engine for mathematical
formulae. It indexes  OpenMath and  MathML formulae, using a technique derived
from automated theorem proving: Substitution Tree Indexing. The software is
licensed under the  GNU General Public License.

Content
-------
* `LICENSE` copy of the license under which this software is distributed
* `TODO` project TODOs which have not materialized into tickets
* `CMakeLists.txt`  cmake build script
* `data/` data used for demo running MWS
* `doc/` documentation and design
* `scripts/` utility scripts
* `src/` source code
* `test/` test source code and data
* `third_party/` third party source code

Compiling and Testing
---------------------
Compiling of the source tree is automated via cmake. You can build the sources
using the following command:

	make

To select which components to compile, use:

	make config

To run the tests, use:

	make test

For more details about these targets, see 3.2

Dependencies
------------
The core executable (mwsd) requires:

  - g++ (with pthread) (>= 4.4)
  - cmake              (>= 2.6)
  - libmicrohttpd      (>= 0.4)
  - make
  - pkg-config
  - libxml2
  - libleveldb
  - libsnappy
  - libjson-c

The Crawler executables (xhtml2harvests) requires:

  - libhtmlcxx-dev
  - libicu-dev
  - libcurl3
  - libcurl4-gnutls-dev

The doc target requires:

  - doxygen

Makefile targets
----------------
* `all` builds all the binaries of the project and tests
* `clean` cleans the build
* `config` brings up the cmake CLI configuration tool
* `doc` generates the documentation of the project
* `test` runs project tests
* `help` display the complete list of targets

Usage
-----
To use the Crawler, one needs to start the crawler with the defined website, 
the count of sites to crawl and optionaly: the start of the crawling and links
to be skipped when crawling.

Another way to generate harvests is via `xhtml2harvests`. This takes as
arguments XHTML documents and crawls them, creating harvests files. To
crawl a repository of XHTML documents, use:

    find . -name *.xhtml | xargs -n 10 xhtml2harvests -O /path/to/store/harvests

The main server will start by running the executable `mwsd`. This takes
as argument a harvest include path which is used to load math data which
will be served on a port via HTTP:

	mwsd -I <harvest include dir> -p <port number>

To setup or remove mwsd as a global SysV service, use (as root):

	scripts/sysv/deploy.sh config/mws_services.conf
	scripts/sysv/remove.sh config/mws_services.conf

The provided [config file](config/mws_services.conf) sets up MathWebSearch to
use the [ZBL demo harvests](data/zbl/). To start/stop services, use

	service mwsd_arxivdemo [start|stop|status|...]

Output is logged to /var/log/mwsd.log. To serve different harvest
paths, create your own configuration file and deploy the service.

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

