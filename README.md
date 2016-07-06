#MariaDB ColumnStore Server (version 1.0)
This is the server part of MariaDB ColumnStore 1.0.1.
MariaDB ColumnStore 1.0.1 is the development version of MariaDB ColumnStore. 
It is built by porting InfiniDB 4.6.7 on MariaDB 10.1.14 and adding entirely 
new features not found anywhere else.

#MariaDB Columnstore Engine (version 1.0)
This is the engine part of MariaDB ColumnStore 1.0.1.
Engine is a submodule of Server in the build process.

###Alpha release notice
MariaDB ColumnStore 1.0.1 is an Alpha release. This is the first MariaDB 
ColumnStore release, not all features planned for the MariaDB ColumnStore 1.0 
series are included in this release. 

A few things to notice:
- Do not use alpha releases on production systems.

Additional features and product enhancements will be pushed in future releases. 

##The structure of this repository is:
* Branch "master" - this is the latest released version of the source code.  Each major release is tagged.
* Branch "develop" - this is the mainline development branch.
* Branch "mcol-xxx" - these are specific bug and feature branches. These are merged into development which is merged to master.

MariaDB columnstore server and the engine are in separate repositories, but the engine repository is integrated into the server repository using a git "sub repository".  The server currently uses CMake but the engine is still based on autotools.

##Build dependencies
These packages need to be install along with the "group development Tools" package:

bison ncurses-develop readline-devel boost-devel perl-devel openssl-devel cmake libxml2-devel

##Building master branch
The current (1.0.1) master branch does not build properly.  This has been rectified in the *development* branch and once version 1.0.2 is released, building the master branch will work correctly.

##Building develop branch

Building can do be as a non-root user. If you do a "build install", it will install the binaries in /usr/local/mariadb/columnstore
and the use of sudo is required.

Currently building has only been certified on CentOS 6.6, 6.7 and 7.0.  Building on other platforms will be certified in a later release.

To build the current development branch
  * git clone https://github.com/mariadb-corporation/mariadb-columnstore-server.git 
  * cd mariadb-columnstore-server
  * git checkout develop        # switch to develop code
  * git submodule update --init # pull in engine code
  * cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/mariadb/columnstore/mysql
  * make -jN                    # N is the number of concurrent build processes and should likely be the number of cores available
  * sudo make install
  * cd mariadb-columnstore-engine
  * ./build/bootstrap
  * ./configure 
  * make -jN                    # same as above with respect to concurrent processes
  * sudo make install
  
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
These packages need to be install:

expect perl perl-DBI openssl zlib file sudo

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

 
