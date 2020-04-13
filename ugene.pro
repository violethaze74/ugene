include(ugene.pri)

use_bundled_zlib() {
    SUBDIRS += src/libs_3rdparty/zlib
}

use_bundled_sqlite() {
    SUBDIRS += src/libs_3rdparty/sqlite3
}

SUBDIRS += \
          src/libs_3rdparty/breakpad \
          src/libs_3rdparty/qscore \
          src/libs_3rdparty/samtools \
          src/libs_3rdparty/QSpec \
          src/corelibs/U2Core \
          src/corelibs/U2Test \
          src/corelibs/U2Algorithm \
          src/corelibs/U2Formats \
          src/corelibs/U2Lang \
          src/corelibs/U2Private \
          src/corelibs/U2Gui \
          src/corelibs/U2View \
          src/corelibs/U2Designer \
          src/corelibs/U2Script \
          src/ugeneui \
          src/ugenecl \
          src/ugenem \
          src/plugins_checker \
          src/plugins_3rdparty/ball \
          src/plugins_3rdparty/sitecon \
          src/plugins_3rdparty/umuscle \
          src/plugins_3rdparty/hmm2 \
          src/plugins_3rdparty/gor4 \
          src/plugins_3rdparty/psipred \
          src/plugins_3rdparty/primer3 \
          src/plugins_3rdparty/phylip \
          src/plugins_3rdparty/kalign \
          src/plugins_3rdparty/ptools \
          src/plugins_3rdparty/variants \
          src/plugins/ngs_reads_classification \
          src/plugins/CoreTests \
          src/plugins/GUITestBase \
          src/plugins/annotator \
          src/plugins/api_tests \
          src/plugins/biostruct3d_view \
          src/plugins/chroma_view \
          src/plugins/circular_view \
          src/plugins/clark_support \
          src/plugins/dbi_bam \
          src/plugins/diamond_support \
          src/plugins/dna_export \
          src/plugins/dna_flexibility \
          src/plugins/dna_graphpack \
          src/plugins/dna_stat \
          src/plugins/dotplot \
          src/plugins/enzymes \
          src/plugins/external_tool_support \
          src/plugins/genome_aligner \
          src/plugins/kraken_support \
          src/plugins/linkdata_support \
          src/plugins/metaphlan2_support \
          src/plugins/orf_marker \
          src/plugins/pcr \
          src/plugins/perf_monitor \
          src/plugins/query_designer \
          src/plugins/remote_blast \
          src/plugins/repeat_finder \
          src/plugins/smith_waterman \
          src/plugins/test_runner \
          src/plugins/weight_matrix \
          src/plugins/wevote_support \
          src/plugins/workflow_designer

use_cuda() {
    SUBDIRS += src/plugins/cuda_support
}

use_opencl() {
    SUBDIRS += src/plugins/opencl_support
}

exclude_list_enabled() {
    SUBDIRS -= src/plugins/CoreTests
    SUBDIRS -= src/plugins/test_runner
    SUBDIRS -= src/plugins/perf_monitor
    SUBDIRS -= src/plugins/GUITestBase
    SUBDIRS -= src/plugins/api_tests
    SUBDIRS -= src/libs_3rdparty/QSpec
}

!equals(GUI_TESTING_ENABLED, 1) {
    SUBDIRS -= src/plugins/GUITestBase
    SUBDIRS -= src/libs_3rdparty/QSpec
}

without_non_free() {
    SUBDIRS -= src/plugins_3rdparty/psipred
}
