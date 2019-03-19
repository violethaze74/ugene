# include (U2View.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2View
include( ../../ugene_lib_common.pri )

QT += xml svg widgets printsupport

useWebKit() {
    QT += webkitwidgets
} else {
    QT += webengine webenginewidgets
}

DEFINES+= QT_FATAL_ASSERT BUILDING_U2VIEW_DLL
LIBS += -L../../$$out_dir()
LIBS += -lU2Core -lU2Algorithm -lU2Formats -lU2Lang -lU2Gui

unix: QMAKE_CXXFLAGS += -Wno-char-subscripts

DESTDIR = ../../$$out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lU2Core -lU2Algorithm -lU2Formats -lU2Lang -lU2Gui
        LIBS += -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Langd -lU2Guid
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

