#!/bin/sh

aclocal
autoheader
automake --include-deps --add-missing --copy 
autoconf

