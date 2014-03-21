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
LICENSE         copy of the license under which this software is distributed
README          this file, containing general information
TODO            file containing present TODOs

CMakeLists.txt  cmake template for the Makefile

data/           data used for static runs (generally xml harvest files)
doc/            documentation and design
scripts/        utilitary scripts to help building and configuring
src/            source code
test/           test sources and data
third_party/    third party sources

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
  - make               (>= 3)
  - pkg-config
  - libxml2            (>= 2.7)
  - libleveldb
  - libsnappy
  - libjson-c

The RESTful interface (restd) requires:

  - libmicrohttpd      (>= 0.4)

The Crawler executables (xhtml2harvests) requires:

  - libhtmlcxx-dev
  - libicu-dev
  - libcurl3
  - libcurl4-gnutls-dev

The doc target requires:

  - doxygen

Makefile targets
----------------
a) all              builds all the binaries of the project and tests
b) clean            clears the binaries of the project and the documentation
c) config           get cmake CLI configuration tool
d) doc              generates the documentation of the project
e) help             display all the targets (main and service)
f) test             runs the unit tests (using ctest)

Usage
-----
The main server will start by running the generated executable "mwsd". The
server will run on port MWS_PORT. The harvest include path is not necessarily
required. All files of MWS_HARVEST_EXT extension are loaded from the respective
paths (without recursion). The respective values are defined in
common/config/MwsDaemon.hpp.
To use the Crawler, one needs to start the crawler with the defined website, 
the count of sites to crawl and optionaly: the start of the crawling and links
to be skipped when crawling.
To be able to use RESTful communication, one needs to also start the REST-Daemon
with the desired RESTful port as argument.

	mwsd -I <harvest include dir>
	restd -p|--port <arg>

To setup or remove mwsd/restd as a global SysV service, use (as root):

	scripts/sysv/deploy.sh config/mws_services.conf
	scripts/sysv/remove.sh config/mws_services.conf

The provided [config file](config/mws_services.conf) sets up MathWebSearch to
use the [ZBL demo harvests](data/zbl/). To start/stop services, use

	service mwsd_arxivdemo [start|stop|status|...]
	service restd_arxivdemo [start|stop|status|...]

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

