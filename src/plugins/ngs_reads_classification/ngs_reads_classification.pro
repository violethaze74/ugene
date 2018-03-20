include (ngs_reads_classification.pri)

# Input
HEADERS += src/ClassificationFilterWorker.h \
           src/DatabaseDelegate.h \
           src/EnsembleClassificationWorker.h \
           src/GenomicLibraryDelegate.h \
           src/GenomicLibraryDialog.h \
           src/GenomicLibraryPropertyWidget.h \
           src/GetReadListWorker.h \
           src/NgsReadsClassificationPlugin.h \
           src/TaxonomySupport.h

SOURCES += src/ClassificationFilterWorker.cpp \
           src/DatabaseDelegate.cpp \
           src/EnsembleClassificationWorker.cpp \
           src/GenomicLibraryDelegate.cpp \
           src/GenomicLibraryDialog.cpp \
           src/GenomicLibraryPropertyWidget.cpp \
           src/GetReadListWorker.cpp \
           src/NgsReadsClassificationPlugin.cpp \
           src/TaxonomySupport.cpp

FORMS += src/GenomicLibraryDialog.ui

TRANSLATIONS += transl/english.ts transl/russian.ts
