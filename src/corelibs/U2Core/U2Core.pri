# include (U2Core.pri)

MODULE_ID=U2Core
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

QT += network xml widgets

DEFINES+=UGENE_MIN_VERSION_SQLITE=$${UGENE_MIN_VERSION_SQLITE}
DEFINES+=UGENE_MIN_VERSION_MYSQL=$${UGENE_MIN_VERSION_MYSQL}
DEFINES+=QT_FATAL_ASSERT BUILDING_U2CORE_DLL

LIBS += $$add_z_lib()
LIBS += $$add_sqlite_lib()

unix: QMAKE_CXXFLAGS += -Wno-char-subscripts

LIBS += -L../../_release

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS += -L../../_debug
        LIBS -= -L../../_release
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

# Special compiler flags for windows configuration
win32 {
    LIBS += User32.lib
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
