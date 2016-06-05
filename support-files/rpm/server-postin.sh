# install SELinux files - but don't override existing ones
SETARGETDIR=/etc/selinux/targeted/src/policy
SEDOMPROG=$SETARGETDIR/domains/program
SECONPROG=$SETARGETDIR/file_contexts/program
if [ -f /etc/redhat-release ] ; then 
   if grep '\(Red Hat Enterprise Linux ..\|CentOS\) release 4' \
     /etc/redhat-release >/dev/null 2>&1; then
     echo
     echo
     echo 'Notes regarding SELinux on this platform:'
     echo '========================================='
     echo
     echo 'The default policy might cause server startup to fail because it is '
     echo 'not allowed to access critical files. In this case, please update '
     echo 'your installation. '
     echo
     echo 'The default policy might also cause inavailability of SSL related '
     echo 'features because the server is not allowed to access /dev/random '
     echo 'and /dev/urandom. If this is a problem, please do the following: '
     echo 
     echo '  1) install selinux-policy-targeted-sources from your OS vendor'
     echo '  2) add the following two lines to '$SEDOMPROG/mysqld.te':'
     echo '       allow mysqld_t random_device_t:chr_file read;'
     echo '       allow mysqld_t urandom_device_t:chr_file read;'
     echo '  3) cd to '$SETARGETDIR' and issue the following command:'
     echo '       make load'
     echo
     echo
  fi
fi

if [ -x sbin/restorecon ] ; then
	sbin/restorecon -R $basemysqldir
fi

