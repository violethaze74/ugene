PLUGIN_ID=disassembly_and_cloning
PLUGIN_NAME=In silico Gene Disassembly and Molecular Cloning
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

QT += webenginewidgets

use_opencl(){
    DEFINES += OPENCL_SUPPORT
}
