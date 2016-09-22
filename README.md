#MariaDB ColumnStore Server (version 1.0)
This is the server part of MariaDB ColumnStore 1.0.3.
MariaDB ColumnStore 1.0.3 is the development version of MariaDB ColumnStore. 
It is built by porting InfiniDB 4.6.7 on MariaDB 10.1.14 and adding entirely 
new features not found anywhere else.

#MariaDB Columnstore Engine (version 1.0)
This is the engine part of MariaDB ColumnStore 1.0.3.
Engine is a submodule of Server in the build process.

###Alpha release notice
MariaDB ColumnStore 1.0.3 is an Alpha release. This is the first MariaDB 
ColumnStore release, not all features planned for the MariaDB ColumnStore 1.0 
series are included in this release. 

Currently building has only been certified on CentOS 6.6, 6.8 and 7.0 and Ubuntu 16.04. 
Building on other platforms will be certified in a later release.

A few things to notice:
- Do not use alpha releases on production systems.

Additional features and product enhancements will be pushed in future releases. 

#Issue tracking
Issue tracking of MariaDB ColumnStore happens in JIRA, https://jira.mariadb.org/browse/MCOL

##The structure of this repository is:
* Branch "master" - this is the latest released version of the source code.  Each major release is tagged.
* Branch "develop" - this is the mainline development branch.
* Branch "mcol-xxx" - these are specific bug and feature branches. These are merged into development which is merged to master.

MariaDB columnstore server and the engine are in separate repositories, but the engine repository is integrated into the server repository using a git "sub repository".  The server currently uses CMake but the engine is still based on autotools.

##Build dependencies

### Boost Libraries
MariaDB Columnstore requires that the boost package of 1.53 or newer is installed for both building and executing

For Centos 7 and Ubuntu 16 and other newer OS's, you can just install the boost packages via yum or apt-get.
For Centos 6 OS's, you will need to install the boost source of 1.55 and build it to generate the required libraries.
So that means both the build and the install machines require this.

yum install boost-devel

NOTE: This means that the "Development Tools" group install be done prior to this.

yum groupinstall "Development Tools"
yum install cmake

Here is the procedure to download and build the boost source:

cd /usr/

wget http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz

tar zxvf boost_1_55_0.tar.gz

cd boost_1_55_0

./bootstrap.sh --with-libraries=atomic,date_time,exception,filesystem,iostreams,locale,program_options,regex,signals,system,test,thread,timer,log --prefix=/usr

./b2 install


### For Centos

These packages need to be install along with the group development packages:

yum groupinstall "Development Tools"
yum install bison ncurses-develop readline-devel perl-devel openssl-devel cmake libxml2-devel

### For Ubuntu 16.04

These packages need to be installed along with the group development packages:

apt-get install build-essential automake libboost-all-dev bison cmake libncurses5-dev libreadline-dev libperl-dev libssl-dev libxml2-dev flex

##Building master branch
The current (1.0.3) master branch is the released version.

##Building develop branch
The develop branch is used for develop updates

Building can be done as a non-root user. If you do a "build install", it will install the binaries in /usr/local/mariadb/columnstore
and the use of sudo is required.

To build the current development branch
  * git clone https://github.com/mariadb-corporation/mariadb-columnstore-server.git 
  * cd mariadb-columnstore-server
  * git checkout develop        # switch to develop code
  * cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/mariadb/columnstore/mysql
  * make -jN                    # N is the number of concurrent build processes and should likely be the number of cores available
  * sudo make install
  * git clone https://github.com/mariadb-corporation/mariadb-columnstore-engine.git
  * cd mariadb-columnstore-engine
  * git checkout develop
  * cmake . 
  * make -jN                    # same as above with respect to concurrent processes
  * sudo make install
  
To build a debug version
  * Add "-DCMAKE_BUILD_TYPE=debug" to each of the cmake commands in the build process
  * Do not mix release and debug versions of server and engine

To develop a new branch/feature/pull request
  * fork the server repo from github mariadb-corporation/mariadb-columnstore-server
  * fork the engine report from github mariadb-corporation/mariadb-columnstore-engine
  * git checkout develop  #branch in server
  * git submodule update --init
  * git branch new-branch-name (this can be in engine or server code)
  * git checkout new-branch-name
  * edit source files
  * git commit -m 'meaningful checkin comment'
  * git push -u origin new-branch-name
  * issue pull request for merge from new-branch-name into develop
  * MariaDB ColumnStore team will evaluate the changes and may request further development or changes before merge 

##Run dependencies
### For Centos

### Boost Libraries
Follow the install procedure for boost from the build Dependecy section above

These packages need to be install:

yum install expect perl perl-DBI openssl zlib file sudo

### For Ubuntu 16.04

These packages need to be installed:

apt-get install expect perl openssl file sudo libdbi-perl libboost-all-dev libreadline-dev

##MariaDB Columnstore utilizes the System Logging for logging purposes
So you will want to make sure that one of these system logging packages is installed:

  syslog, rsyslog, or syslog-ng

##Configure and Starting of MariaDB Columnstore 

Follow the binary package install instructions in the Columnstore Getting Starter Guide:

  https://mariadb.com/kb/en/mariadb/columnstore-getting-started/

Commands to run as root user:

  * cd /usr/local/mariadb/columnstore/bin/
  * ./post-install
  * ./postConfigure

 
