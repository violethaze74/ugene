include (7z.pri)

# Input
HEADERS += src/7z.h \
           src/7zAlloc.h \
           src/7zBuf.h \
           src/7zCrc.h \
           src/7zFile.h \
           src/7zTypes.h \
           src/7zVersion.h \
           src/Bcj2.h \
           src/Bra.h \
           src/Compiler.h \
           src/CpuArch.h \
           src/Delta.h \
           src/Lzma2Dec.h \
           src/LzmaDec.h \
           src/Ppmd.h \
           src/Ppmd7.h \
           src/Precomp.h

SOURCES += src/7zAlloc.c \
           src/7zArcIn.c \
           src/7zBuf.c \
           src/7zBuf2.c \
           src/7zCrc.c \
           src/7zCrcOpt.c \
           src/7zDec.c \
           src/7zFile.c \
           src/7zStream.c \
           src/Bcj2.c \
           src/Bra.c \
           src/Bra86.c \
           src/BraIA64.c \
           src/CpuArch.c \
           src/Delta.c \
           src/Lzma2Dec.c \
           src/LzmaDec.c \
           src/Ppmd7.c \
           src/Ppmd7Dec.c
