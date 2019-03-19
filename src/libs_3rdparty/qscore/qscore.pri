# include (qscore.pri)
include( ../../ugene_globals.pri )
UGENE_RELATIVE_DESTDIR = ''

TEMPLATE = lib
CONFIG +=qt thread debug_and_release staticlib warn_off
QT += network xml script
INCLUDEPATH += src _tmp ../../core/src  ../../include

LIBS += -L../../$$out_dir()
LIBS += -lU2Core -lU2Core -lcore
DESTDIR = ../../$$out_dir()

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = qscored
        DEFINES+=_DEBUG
        CONFIG +=console
        MOC_DIR=_tmp/moc/debug
        OBJECTS_DIR=_tmp/obj/debug
        LIBS -= -lU2Core -lU2Core -lcore
        LIBS += -lcored -lU2Cored -lU2Cored
    }

    CONFIG(release, debug|release) {
        TARGET = qscore
        DEFINES+=NDEBUG
        MOC_DIR=_tmp/moc/release
        OBJECTS_DIR=_tmp/obj/release
    }
}


UI_DIR=_tmp/ui
RCC_DIR=_tmp/rcc

win32 {
    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON = -W3

    QMAKE_MSVC_PROJECT_NAME=lib_3rd_qscore

    LIBS += psapi.lib
}

macx {
    QMAKE_RPATHDIR += @executable_path/
    QMAKE_LFLAGS_SONAME = -Wl,-dylib_install_name,@rpath/
}

#unix {
#    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
#    INSTALLS += target
#}
