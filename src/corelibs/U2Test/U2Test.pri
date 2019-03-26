# include (U2Test.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2Test
include( ../../ugene_lib_common.pri )

QT += xml gui widgets
DEFINES+= QT_FATAL_ASSERT BUILDING_U2TEST_DLL

LIBS += -L../../$$out_dir()
LIBS += -lU2Core -lQSpec
INCLUDEPATH += ../../libs_3rdparty/QSpec/src

if (contains(DEFINES, HI_EXCLUDED)) {
    LIBS -= -lQSpec
}

DESTDIR = ../../$$out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lU2Core -lQSpec
        LIBS += -lU2Cored -lQSpecd
        if (contains(DEFINES, HI_EXCLUDED)) {
            LIBS -= -lQSpecd
        }
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

