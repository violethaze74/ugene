set(UGENE_PLUGIN_NAME diamond_support)

include(../../Plugin.cmake)

target_link_libraries(diamond_support ngs_reads_classification)

