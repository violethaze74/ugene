# include (U2Lang.pri)

MODULE_ID=U2Lang
include( ../../ugene_lib_common.pri )
UGENE_RELATIVE_DESTDIR = ''

QT += xml widgets
DEFINES+= QT_FATAL_ASSERT BUILDING_U2LANG_DLL

LIBS += -L../../../$$corelibs_out_dir()
LIBS += -lU2Core
DESTDIR = ../../../$$corelibs_out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS += -lU2Cored
        LIBS -= -lU2Core
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
