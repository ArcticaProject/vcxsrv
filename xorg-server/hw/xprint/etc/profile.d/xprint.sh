#
# /etc/profile.d/xprint.sh
#
# Copyright (c) 2002-2004 by Roland Mainz <roland.mainz@nrubsig.org>
# please send bugfixes or comments to http://xprint.mozdev.org/

#
# Obtain list of Xprint servers
#

if [ -f "/etc/init.d/xprint" ] ; then
  XPSERVERLIST="`/bin/sh /etc/init.d/xprint get_xpserverlist`"
  export XPSERVERLIST
fi

# /etc/profile.d/xprint.sh ends here.
