# include (U2Script.pri)

MODULE_ID=U2Script

include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

DEFINES +=          QT_FATAL_ASSERT BUILDING_U2SCRIPT_DLL

QT += network xml widgets

INCLUDEPATH +=      ../../include \
                    ../U2Private/src

#count( UGENE_NODE_DIR, 1 ) {
#    QMAKE_EXTENSION_SHLIB = node
#
#    INCLUDEPATH +=  $${UGENE_NODE_DIR}/src \
#                    $${UGENE_NODE_DIR}/deps/v8/include \
#                    $${UGENE_NODE_DIR}/deps/uv/include
#}

LIBS += -L../../$$out_dir()
LIBS += -lU2Core -lU2Algorithm -lU2Formats -lU2Lang -lU2Private -lU2Gui -lU2Test
LIBS += $$add_sqlite_lib()

DESTDIR = ../../$$out_dir()

!debug_and_release|build_pass {

    CONFIG( debug, debug|release ) {
        LIBS -= -lU2Core -lU2Algorithm -lU2Formats -lU2Lang -lU2Private -lU2Gui -lU2Test
        LIBS += -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Langd -lU2Privated -lU2Guid -lU2Testd
    }
}

unix {
    target.path =   $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS +=     target
}
