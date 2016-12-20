IF(DEB)

SET(CMAKE_INSTALL_PREFIX "/usr/local/mariadb/columnstore/mysql")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_DEBUG 1)
SET(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)


SET(CPACK_DEB_COMPONENT_INSTALL ON)

SET(CPACK_COMPONENT_SERVER_GROUP "server")
SET(CPACK_COMPONENT_MANPAGESSERVER_GROUP "server")
SET(CPACK_COMPONENT_INIFILES_GROUP "server")
SET(CPACK_COMPONENT_SERVER_SCRIPTS_GROUP "server")
SET(CPACK_COMPONENT_SUPPORTFILES_GROUP "server")
SET(CPACK_COMPONENT_DEVELOPMENT_GROUP "devel")
SET(CPACK_COMPONENT_MANPAGESDEVELOPMENT_GROUP "devel")
SET(CPACK_COMPONENT_TEST_GROUP "test")
SET(CPACK_COMPONENT_MANPAGESTEST_GROUP "test")
SET(CPACK_COMPONENT_CLIENT_GROUP "client")
SET(CPACK_COMPONENT_MANPAGESCLIENT_GROUP "client")
SET(CPACK_COMPONENT_README_GROUP "server")
SET(CPACK_COMPONENT_SHAREDLIBRARIES_GROUP "shared")
SET(CPACK_COMPONENT_COMMON_GROUP "common")
SET(CPACK_COMPONENT_CLIENTPLUGINS_GROUP "common")
SET(CPACK_COMPONENT_COMPAT_GROUP "compat")
SET(CPACK_COMPONENTS_ALL Server ManPagesServer IniFiles Server_Scripts
                         SupportFiles Development ManPagesDevelopment
                         ManPagesTest Readme ManPagesClient Test 
                         Common Client SharedLibraries ClientPlugins)

## dhill
SET(INFINIDB_DEBIAN_PACKAGE_NAME "mariadb-columnstore")
SET(INFINIDB_BIT "x86_64")

IF (NOT CPACK_DEBIAN_PACKAGE_VERSION)
SET (CPACK_DEBIAN_PACKAGE_VERSION ${PACKAGE_VERSION})
ENDIF()
IF (NOT CPACK_DEBIAN_PACKAGE_RELEASE)
SET (CPACK_DEBIAN_PACKAGE_RELEASE ${PACKAGE_RELEASE})
ENDIF()

SET(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
SET(CPACK_PACKAGE_FILE_NAME "${INFINIDB_DEBIAN_PACKAGE_NAME}-${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}-${INFINIDB_BIT}-${DEB}")
SET(COLUMNSTORE_NO_DASH_VERSION ${CPACK_DEBIAN_PACKAGE_VERSION})

SET(CPACK_DEBIAN_PACKAGE_LICENSE "GPLv2")
SET(CPACK_DEBIAN_PACKAGE_RELOCATABLE FALSE)
SET(CPACK_PACKAGE_RELOCATABLE FALSE)
SET(CPACK_DEBIAN_PACKAGE_GROUP "Applications/Databases")
SET(CPACK_DEBIAN_PACKAGE_SUMMARY ${CPACK_PACKAGE_SUMMARY})
SET(CPACK_DEBIAN_PACKAGE_URL ${CPACK_PACKAGE_URL})

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "mariadb-columnstore-common")


SET(CPACK_DEBIAN_CLIENT_PACKAGE_REPLACES "mysql-client, MySQL-client, MySQL-OurDelta-client")
SET(CPACK_DEBIAN_CLIENT_PACKAGE_PROVIDES "MySQL-client, mysql-client")

SET(CPACK_DEBIAN_DEVEL_PACKAGE_REPLACES "MySQL-devel, MySQL-OurDelta-devel")
SET(CPACK_DEBIAN_DEVEL_PACKAGE_PROVIDES "MySQL-devel")

SET(CPACK_DEBIAN_SERVER_PACKAGE_REPLACES "MariaDB, MySQL, mysql-server, MySQL-server, MariaDB-Galera-server, MySQL-OurDelta-server")
SET(CPACK_DEBIAN_SERVER_PACKAGE_PROVIDES "MariaDB, MySQL, MySQL-server, msqlormysql, mysql-server")

SET(CPACK_DEBIAN_SHARED_PACKAGE_REPLACES "mysql-shared, MySQL-shared-standard, MySQL-shared-pro, MySQL-shared-pro-cert, MySQL-shared-pro-gpl, MySQL-shared-pro-gpl-cert, MySQL-shared, MySQL-OurDelta-shared")
SET(CPACK_DEBIAN_SHARED_PACKAGE_PROVIDES "MySQL-shared, mysql-shared")

SET(CPACK_DEBIAN_TEST_PACKAGE_REPLACES "MySQL-test, MySQL-OurDelta-test")
SET(CPACK_DEBIAN_TEST_PACKAGE_PROVIDES "MySQL-test")

SET(CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, mariadb-columnstore-client")

IF(WITH_WSREP)
SET(CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "${CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS}, galera, rsync, lsof, grep, gawk, iproute, coreutils, findutils, tar")
ENDIF()

set( CPACK_DEBIAN_SERVER_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/support-files/debian/server/postinst;${CMAKE_CURRENT_SOURCE_DIR}/support-files/debian/server/prerm;${CMAKE_CURRENT_SOURCE_DIR}/support-files/debian/server/postrm;" )
#set( CPACK_DEBIAN_SERVER_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/mariadb-server-10.1.preinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/mariadb-server-10.1.postinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/mariadb-server-10.1.prerm;${CMAKE_CURRENT_SOURCE_DIR}/debian/mariadb-server-10.1.postrm;" )
set( CPACK_DEBIAN_SHARED_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/support-files/debian/shared/postinst;${CMAKE_CURRENT_SOURCE_DIR}/support-files/debian/shared/postrm;" )


MACRO(ALTERNATIVE_NAME real alt)
  SET(ver "${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}")
  IF (${epoch})
    SET(ver "${epoch}:${ver}")
  ENDIF()

  SET(p "CPACK_DEBIAN_${real}_PACKAGE_PROVIDES")
  SET(${p} "${${p}}, ${alt} (= ${ver}), ${alt}${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} (= ${ver})")
  SET(o "CPACK_DEBIAN_${real}_PACKAGE_REPLACES")
  SET(${o} "${${o}}, ${alt}, ${alt}${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ENDMACRO(ALTERNATIVE_NAME)

ALTERNATIVE_NAME("DEVEL"  "mysql-devel")
ALTERNATIVE_NAME("SERVER" "mysql-server")
ALTERNATIVE_NAME("TEST"   "mysql-test")

# Argh! Different distributions call packages differently, to be a drop-in
# replacement we have to fake distribution-speficic dependencies

IF(DEBIAN MATCHES "(rhel|centos)5")
  ALTERNATIVE_NAME("SHARED" "mysql")
ELSEIF(DEBIAN MATCHES "(rhel|centos)6")
  ALTERNATIVE_NAME("CLIENT" "mysql")
  ALTERNATIVE_NAME("SHARED" "mysql-libs")
ELSEIF(DEBIAN MATCHES "fedora" OR DEBIAN MATCHES "(rhel|centos)7")
  SET(epoch 1) # this is fedora
  ALTERNATIVE_NAME("CLIENT" "mariadb")
  ALTERNATIVE_NAME("CLIENT" "mysql")
  ALTERNATIVE_NAME("DEVEL"  "mariadb-devel")
  ALTERNATIVE_NAME("SERVER" "mariadb-server")
  ALTERNATIVE_NAME("SERVER" "mysql-compat-server")
  ALTERNATIVE_NAME("SHARED" "mysql-libs")
  ALTERNATIVE_NAME("TEST"   "mariadb-test")
ENDIF()

SET(CPACK_DEBIAN_TEST_PACKAGE_PROVIDES "perl")

# If we want to build build MariaDB-shared-compat,
# extract compat libraries from MariaDB-shared-5.3 deb
FILE(GLOB compat_deb RELATIVE ${CMAKE_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/../MariaDB-shared-5.3.*.deb")
IF (compat_deb)
  MESSAGE("Using ${compat_deb} to build MariaDB-compat")
  INSTALL(CODE "EXECUTE_PROCESS(
                 COMMAND rpm2cpio ${CMAKE_SOURCE_DIR}/${compat_deb}
                 COMMAND cpio --extract --make-directories */libmysqlclient*.so.* -
                 WORKING_DIRECTORY \$ENV{DESTDIR})
                EXECUTE_PROCESS(
                 COMMAND chmod -R a+rX .
                 WORKING_DIRECTORY \$ENV{DESTDIR})"
                 COMPONENT Compat)
  SET(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} Compat)

  # RHEL6/CentOS6 install Postfix by default, and it requires
  # libmysqlclient.so.16 that pulls in mysql-libs-5.1.x
  # And the latter conflicts with our rpms.
  # Make sure that for these distribuions all our rpms require
  # MariaDB-compat, that will replace mysql-libs-5.1
  IF(DEBIAN MATCHES "(rhel|centos)6")
    SET(CPACK_DEBIAN_COMMON_PACKAGE_DEPENDS "MariaDB-compat")
  ENDIF()
ENDIF(compat_deb)

SET(CPACK_DEBIAN_COMPAT_PACKAGE_PROVIDES "mysql-libs = 5.3.5") # exact version doesn't matter as long as it greater than 5.1
SET(CPACK_DEBIAN_COMPAT_PACKAGE_REPLACES "mysql-libs < 5.3.5")

ENDIF(DEB)

