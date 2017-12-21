include (ngs_reads_classification.pri)

# Input
HEADERS += src/GenomicLibraryDelegate.h \
           src/GenomicLibraryDialog.h \
           src/GenomicLibraryPropertyWidget.h \
           src/NgsReadsClassificationPlugin.h

SOURCES += src/GenomicLibraryDelegate.cpp \
           src/GenomicLibraryDialog.cpp \
           src/GenomicLibraryPropertyWidget.cpp \
           src/NgsReadsClassificationPlugin.cpp

FORMS += src/GenomicLibraryDialog.ui

TRANSLATIONS += transl/english.ts transl/russian.ts
