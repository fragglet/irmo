#!/bin/sh

aclocal
libtoolize
autoheader
automake -a 
autoconf
automake -a

./configure $@

