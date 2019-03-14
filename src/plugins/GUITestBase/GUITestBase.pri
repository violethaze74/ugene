#include (GUITestBase.pri)

PLUGIN_ID=GUITestBase
PLUGIN_NAME=GUI Test Base
PLUGIN_VENDOR=Unipro
include( ../../ugene_plugin_common.pri )

QT += testlib webkitwidgets

INCLUDEPATH += ../../corelibs/U2View/_tmp/ ../../libs_3rdparty/QSpec/src
LIBS += -L../../../$$corelibs_out_dir()
LIBS += -lQSpec

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        LIBS -= -lQSpec
        LIBS += -lQSpecd
    }
}

unix {
    !macx {
    LIBS += -lXtst
    }
    macx {
    QMAKE_LFLAGS += -framework ApplicationServices
    }
}

win32 {
    LIBS += User32.lib Gdi32.lib
    QMAKE_CXXFLAGS += -bigobj
}

macx {
    LIBS += -framework AppKit
}



