include (kraken_support.pri)

# Input
HEADERS += src/DatabaseSizeRelation.h \
           src/DatabaseValidator.h \
           src/KrakenBuildPrompter.h \
           src/KrakenBuildTask.h \
           src/KrakenBuildWorker.h \
           src/KrakenBuildWorkerFactory.h \
           src/KrakenClassifyLogParser.h \
           src/KrakenClassifyPrompter.h \
           src/KrakenClassifyTask.h \
           src/KrakenClassifyWorker.h \
           src/KrakenClassifyWorkerFactory.h \
           src/KrakenSupport.h \
           src/KrakenSupportPlugin.h \
           src/KrakenTranslateLogParser.h \
           src/MinimizerLengthValidator.h

SOURCES += src/DatabaseSizeRelation.cpp \
           src/DatabaseValidator.cpp \
           src/KrakenBuildPrompter.cpp \
           src/KrakenBuildTask.cpp \
           src/KrakenBuildWorker.cpp \
           src/KrakenBuildWorkerFactory.cpp \
           src/KrakenClassifyLogParser.cpp \
           src/KrakenClassifyPrompter.cpp \
           src/KrakenClassifyTask.cpp \
           src/KrakenClassifyWorker.cpp \
           src/KrakenClassifyWorkerFactory.cpp \
           src/KrakenSupport.cpp \
           src/KrakenSupportPlugin.cpp \
           src/KrakenTranslateLogParser.cpp \
           src/MinimizerLengthValidator.cpp

TRANSLATIONS += transl/russian.ts

