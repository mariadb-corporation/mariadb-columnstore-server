# install SELinux files - but don't override existing ones
SETARGETDIR=/etc/selinux/targeted/src/policy
SEDOMPROG=$SETARGETDIR/domains/program
SECONPROG=$SETARGETDIR/file_contexts/program

if [ -x sbin/restorecon ] ; then
	sbin/restorecon -R /usr/local/mariadb/columnstore/mysql
fi

