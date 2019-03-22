# include (diamond_support.pri)
include (../../ugene_version.pri)

PLUGIN_ID=diamond_support
PLUGIN_NAME=DIAMOND external tool support
PLUGIN_VENDOR=Unipro
PLUGIN_DEPENDS=ngs_reads_classification:$${UGENE_VERSION};external_tool_support:$${UGENE_VERSION}

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        PLUGIN_DEPENDS -= ngs_reads_classification:$${UGENE_VERSION};external_tool_support:$${UGENE_VERSION}
        PLUGIN_DEPENDS += ngs_reads_classificationd:$${UGENE_VERSION};external_tool_supportd:$${UGENE_VERSION}
    }
}

include( ../../ugene_plugin_common.pri )

LIBS += -L../../$$out_dir()/plugins
LIBS += -lngs_reads_classification

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        LIBS -= -lngs_reads_classification
        LIBS += -lngs_reads_classificationd
    }
}
