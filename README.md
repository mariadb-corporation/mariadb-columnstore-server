## MariaDB: drop-in replacement for MySQL

MariaDB is designed as a drop-in replacement of MySQL(R) with more
features, new storage engines, fewer bugs, and better performance.

MariaDB is brought to you by the MariaDB Foundation.
Please read the CREDITS file for details about the MariaDB Foundation,
and who is developing MariaDB.

MariaDB is developed by many of the original developers of MySQL who
now work for the MariadB Foundation and the MariaDB Corporation, and by many people in
the community.

MySQL, which is the base of MariaDB, is a product and trademark of Oracle
Corporation, Inc. For a list of developers and other contributors,
see the Credits appendix.  You can also run 'SHOW authors' to get a
list of active contributors.

A description of the MariaDB project and a manual can be found at:
http://mariadb.org/
https://mariadb.com/kb/en/
https://mariadb.com/kb/en/mariadb-vs-mysql-features/
https://mariadb.com/kb/en/mariadb-versus-mysql-features/
https://mariadb.com/kb/en/mariadb-versus-mysql-compatibility/

As MariaDB is a full replacement of MySQL, the MySQL manual at
http://dev.mysql.com/doc is generally applicable.

Help:
-----

More help is available from the Maria Discuss mailing list
https://launchpad.net/~maria-discuss
and the #maria IRC channel on Freenode.


License:
--------

***************************************************************************

NOTE: 

MariaDB is specifically available only under version 2 of the GNU
General Public License (GPLv2). (I.e. Without the "any later version"
clause.) This is inherited from MySQL. Please see the README file in
the MySQL distribution for more information.

License information can be found in the COPYING, COPYING.LESSER,
and COPYING.thirdparty files.

***************************************************************************

Bug Reports:
------------

Bug and/or error reports regarding MariaDB should be submitted at
http://mariadb.org/jira

For CentOS 6, you can either download and install the MariaDB Columnstore Centos 6 boost library package or install the boost source of 1.55 and build it to generate the required libraries. That means both the build and the install machines require this.

Downloading and installing the MariaDB Columnstore Centos 6 boost library package is documented here:

https://mariadb.com/kb/en/mariadb/preparing-for-columnstore-installation/#boost-libraries

Bugs in the MySQL code can also be submitted at http://bugs.mysql.com

The code for MariaDB, including all revision history, can be found at:
https://github.com/MariaDB/server

***************************************************************************

Downloading and build the boost libraries:
------------------------------------------

Here is the procedure to download and build the boost source:

```bash
cd /usr/

wget http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz

tar zxvf boost_1_55_0.tar.gz

cd boost_1_55_0

./bootstrap.sh --with-libraries=atomic,date_time,exception,filesystem,iostreams,locale,program_options,regex,signals,system,test,thread,timer,log --prefix=/usr

./b2 install

ldconfig
```
``

### For CentOS 7

These packages need to be install along with the group development packages:

```bash
yum groupinstall "Development Tools"
yum install bison ncurses-devel readline-devel perl-devel openssl-devel cmake libxml2-devel snappy-devel
```


### For Ubuntu 16

These packages need to be installed along with the group development packages:

```bash
apt-get install build-essential automake libboost-all-dev bison cmake libncurses5-dev libreadline-dev libperl-dev libssl-dev libxml2-dev flex libsnappy-dev
```
### For Debian 8

These packages need to be installed along with the group development packages:

```bash
apt-get install build-essential automake libboost-all-dev bison cmake libncurses5-dev libreadline-dev libperl-dev libssl-dev libxml2-dev flex libsnappy-dev
```

##Building master branch
The current master branch is the released version.

##Building develop branch
The develop branch is used for develop updates

Building can be done as a non-root user. If you do a "build install", it will install the binaries in `/usr/local/mariadb/columnstore`
and the use of sudo is required.

To build the current development branch (Engine checkout inside Server):
```bash
git clone https://github.com/mariadb-corporation/mariadb-columnstore-server.git
cd mariadb-columnstore-server
git checkout develop # switch to develop code
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/mariadb/columnstore/mysql
make -jN # N is the number of concurrent build processes and should likely be the number of cores available
sudo make install
git clone https://github.com/mariadb-corporation/mariadb-columnstore-engine.git
cd mariadb-columnstore-engine
git checkout develop
cmake .
make -jN # same as above with respect to concurrent processes
sudo make install
```

With the engine checked out in a separate location the following values need to be set by cmake command.

```bash
SERVER_BUILD_INCLUDE_DIR=Path to the server build include directory.
SERVER_SOURCE_ROOT_DIR=Path the directory the server source checked out from github.
```

###Examples
Engine not located inside server:

```bash
git clone https://github.com/mariadb-corporation/mariadb-columnstore-server.git
git clone https://github.com/mariadb-corporation/mariadb-columnstore-engine.git
cd mariadb-columnstore-server
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/mariadb/columnstore/mysql
make -jN # N is the number of concurrent build processes and should likely be the number of cores available
sudo make install
cd ../mariadb-columnstore-engine
cmake . -DSERVER_BUILD_INCLUDE_DIR=../mariadb-columnstore-server/include -DSERVER_SOURCE_ROOT_DIR=../mariadb-columnstore-server
make -jN # same as above with respect to concurrent processes
sudo make install
```

Build out-of-source:

```bash
git clone https://github.com/mariadb-corporation/mariadb-columnstore-server.git
git clone https://github.com/mariadb-corporation/mariadb-columnstore-engine.git
mkdir buildServer
cd buildServer
cmake ../mariadb-columnstore-server -DCMAKE_INSTALL_PREFIX=/usr/local/mariadb/columnstore/mysql
make -jN # N is the number of concurrent build processes and should likely be the number of cores available
sudo make install
cd ..
mkdir buildEngine
cd buildEngine
cmake ../mariadb-columnstore-engine -DSERVER_BUILD_INCLUDE_DIR=../buildServer/include -DSERVER_SOURCE_ROOT_DIR=../mariadb-columnstore-server
make -jN # same as above with respect to concurrent processes
sudo make install
```

To build a debug version
  * Add `-DCMAKE_BUILD_TYPE=debug` to each of the cmake commands in the build process
  * Do not mix release and debug versions of server and engine

To develop a new branch/feature/pull request
  * Fork the server repo from github `mariadb-corporation/mariadb-columnstore-server`
  * Fork the engine report from github `mariadb-corporation/mariadb-columnstore-engine`
  * `git checkout develop  #branch in server`
  * `git submodule update --init`
  * `git branch new-branch-name` (this can be in engine or server code)
  * `git checkout new-branch-name`
  * Edit source files
  * `git commit -m 'meaningful checkin comment'`
  * `git push -u origin new-branch-name`
  * Issue pull request for merge from new-branch-name into develop
  * MariaDB ColumnStore team will evaluate the changes and may request further development or changes before merge 

##Run dependencies
## For CentOS

For CentOS 6 follow the install procedure for boost from the build Dependecy section above, with CentOS 7 you can just do:

```bash
yum install boost
```

In addition these packages need to be install:

```bash
yum install expect perl perl-DBI openssl zlib file sudo perl-DBD-MySQL libaio snappy
```

## For Ubuntu 16

These packages need to be installed:

```bash
apt-get install expect perl openssl file sudo libdbi-perl libboost-all-dev libreadline-dev libsnappy1
```

## For Debian 8

These packages need to be installed:

```bash
apt-get install expect perl openssl file sudo libdbi-perl libboost-all-dev libreadline-dev libsnappy1
```

##MariaDB ColumnStore utilizes the System Logging for logging purposes
So you will want to make sure that one of these system logging packages is installed:

  syslog, rsyslog, or syslog-ng

##Configure and Starting of MariaDB ColumnStore

Follow the binary package install instructions in the ColumnStore Getting Starter Guide:

  https://mariadb.com/kb/en/mariadb/columnstore-getting-started/

Commands to run as root user:

```bash
cd /usr/local/mariadb/columnstore/bin/
./post-install
./postConfigure
```
=======
Code status:
------------
>>>>>>> MCOL-597

* [![tests status](https://secure.travis-ci.org/MariaDB/server.png?branch=10.2)](https://travis-ci.org/MariaDB/server) travis-ci.org (10.2 branch)
