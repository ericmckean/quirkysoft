
LIBPNG=libpng-1.2.31
ZLIB=zlib-1.2.3
GIFLIB=giflib-4.1.6
JPEGLIB=jpegsrc.v6b

WGET=wget -q

all: ${LIBPNG} ${ZLIB} ${GIFLIB} ${JPEGLIB}

${LIBPNG}.tar.bz2:
	@echo get $@
	@$(WGET) ftp://ftp.simplesystems.org/pub/libpng/png/src/${LIBPNG}.tar.bz2

${ZLIB}.tar.gz:
	@echo get $@
	@$(WGET) http://www.zlib.net/${ZLIB}.tar.gz

${GIFLIB}.tar.gz:
	@echo get $@
	@$(WGET) http://downloads.sourceforge.net/giflib/${GIFLIB}.tar.gz 

${JPEGLIB}.tar.gz:
	@echo get $@
	@$(WGET) http://quirkysoft.googlecode.com/files/${JPEGLIB}.tar.gz

$(LIBPNG) : $(LIBPNG).tar.bz2
	tar xzf $<
