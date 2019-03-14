# include (U2Formats.pri)

MODULE_ID=U2Formats
include( ../../ugene_lib_common.pri )

LIBS += $$add_z_lib()

UGENE_RELATIVE_DESTDIR = ''

DEFINES += QT_FATAL_ASSERT BUILDING_U2FORMATS_DLL

LIBS += -L../../../$$corelibs_out_dir()
LIBS += -lU2Core -lU2Algorithm
LIBS += -lsamtools
LIBS += $$add_sqlite_lib()

win32-msvc2013 {
    DEFINES += NOMINMAX _XKEYCHECK_H
}

QT += sql widgets

# Force re-linking when lib changes
unix:POST_TARGETDEPS += ../../../_release/libsamtools.a
# Same options which samtools is built with
DEFINES+="_FILE_OFFSET_BITS=64" _LARGEFILE64_SOURCE _USE_KNETFILE
INCLUDEPATH += ../../libs_3rdparty/samtools/src ../../libs_3rdparty/samtools/src/samtools
win32:INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/win32
win32:LIBS += -lws2_32
win32:DEFINES += _USE_MATH_DEFINES "__func__=__FUNCTION__" "R_OK=4" "atoll=_atoi64" "alloca=_alloca"

win32 {
    # not visual studio 2015
    !win32-msvc2015 {
        DEFINES += "inline=__inline"
    }
}

INCLUDEPATH += ../../libs_3rdparty/sqlite3/src
DESTDIR = ../../../$$corelibs_out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lU2Core -lU2Algorithm -lsamtools
        LIBS += -lU2Cored -lU2Algorithmd -lsamtoolsd

        unix:POST_TARGETDEPS -= ../../../_release/libsamtools.a
        unix:POST_TARGETDEPS += ../../../_debug/libsamtoolsd.a
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
