if [ $1 = 0 ] ; then
	# Stop MySQL before uninstalling it
	if [ -x %{_sysconfdir}/init.d/mysql-Columnstore ] ; then
		%{_sysconfdir}/init.d/mysql-columnstore stop > /dev/null
	fi
fi

# We do not remove the mysql user since it may still own a lot of
# database files.

