set(UGENE_PLUGIN_NAME kraken_support)

include(../../Plugin.cmake)

target_link_libraries(kraken_support ngs_reads_classification)

