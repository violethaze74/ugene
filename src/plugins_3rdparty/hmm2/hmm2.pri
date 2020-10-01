PLUGIN_ID=hmm2
PLUGIN_NAME=HMM2
PLUGIN_VENDOR=Unipro
CONFIG += warn_off
include( ../../ugene_plugin_common.pri )

win32 {
    QMAKE_CXXFLAGS+=/wd4244 /wd4305
}
