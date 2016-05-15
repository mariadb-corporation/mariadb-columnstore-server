if [ $1 -ge 1 ]; then
  if [ -x %{_sysconfdir}/init.d/mysql-Columnstore ] ; then
    # only restart the server if it was alredy running
    if %{_sysconfdir}/init.d/mysql-Columnstore status > /dev/null 2>&1; then
      %{_sysconfdir}/init.d/mysql-Columnstore restart
    fi
  fi
fi

if [ $1 = 0 ] ; then
  if [ -x /usr/bin/systemctl ] ; then
    /usr/bin/systemctl daemon-reload > /dev/null 2>&1
  fi
fi

