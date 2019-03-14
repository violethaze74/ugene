# include (U2Designer.pri)

MODULE_ID=U2Designer
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

QT += svg
useWebKit() {
    QT += webkitwidgets
} else {
    QT += webengine webenginewidgets
}

DEFINES+= QT_FATAL_ASSERT BUILDING_U2DESIGNER_DLL

LIBS += -L../../../$$corelibs_out_dir()
LIBS += -lU2Core -lU2Lang -lU2Gui
DESTDIR = ../../../$$corelibs_out_dir()

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        LIBS -= -lU2Core -lU2Lang -lU2Gui
        LIBS += -lU2Cored -lU2Langd -lU2Guid
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
