#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

intltoolize

autoreconf -v --install || exit 1
cd $ORIGDIR || exit $?

$srcdir/configure --enable-maintainer-mode "$@"
