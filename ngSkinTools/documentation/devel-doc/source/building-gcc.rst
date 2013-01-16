Installing GCC-4.1.2
====================

Super-brief explanation how to install the right compiler on Linux.


* download gcc-4.1.2.tar.gz 
* create separate dir for building
* configure:
	../gcc-4.1.2/configure --prefix=/opt/gcc412 --program-suffix=412 --enable-languages=c,c++ --enable-shared --enable-threads=posix --disable-checking --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --disable-multilib
* make
* make install
	