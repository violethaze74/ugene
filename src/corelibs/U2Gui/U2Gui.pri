# include (U2Gui.pri)

MODULE_ID=U2Gui
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

QT += network xml svg sql widgets printsupport
DEFINES+= QT_FATAL_ASSERT BUILDING_U2GUI_DLL
INCLUDEPATH += ../U2Private/src

useWebKit() {
    QT += webkitwidgets
} else {
    QT += webenginewidgets websockets webchannel
}
LIBS += -L../../../$$corelibs_out_dir()
LIBS += -lU2Core -lU2Formats -lU2Private

DESTDIR = ../../../$$corelibs_out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lU2Core -lU2Formats -lU2Private
        LIBS += -lU2Cored -lU2Formatsd -lU2Privated
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
