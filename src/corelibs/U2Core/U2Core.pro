include (U2Core.pri)

# Input
HEADERS += src/cmdline/CMDLineCoreOptions.h \
           src/cmdline/CMDLineHelpProvider.h \
           src/cmdline/CMDLineRegistry.h \
           src/cmdline/CMDLineUtils.h \
           src/datatype/Annotation.h \
           src/datatype/AnnotationData.h \
           src/datatype/AnnotationGroup.h \
           src/datatype/AnnotationModification.h \
           src/datatype/AnnotationSettings.h \
           src/datatype/AnnotationTableObjectConstraints.h \
           src/datatype/BioStruct3D.h \
           src/datatype/DIProperties.h \
           src/datatype/DNAAlphabet.h \
           src/datatype/DNAAlphabetRegistryImpl.h \
           src/datatype/DNAChromatogram.h \
           src/datatype/DNAInfo.h \
           src/datatype/DNAQuality.h \
           src/datatype/DNASequence.h \
           src/datatype/DNASequenceUtils.h \
           src/datatype/DNATranslation.h \
           src/datatype/DNATranslationImpl.h \
           src/datatype/FeatureColors.h \
           src/datatype/Matrix44.h \
           src/datatype/PFMatrix.h \
           src/datatype/PhyTree.h \
           src/datatype/PWMatrix.h \
           src/datatype/SMatrix.h \
           src/datatype/U2Alphabet.h \
           src/datatype/U2Assembly.h \
           src/datatype/U2Attribute.h \
           src/datatype/U2CoreAttributes.h \
           src/datatype/U2Feature.h \
           src/datatype/U2FeatureType.h \
           src/datatype/U2Identity.h \
           src/datatype/U2IdTypes.h \
           src/datatype/U2Location.h \
           src/datatype/U2Mod.h \
           src/datatype/U2Qualifier.h \
           src/datatype/U2Range.h \
           src/datatype/U2RawData.h \
           src/datatype/U2Region.h \
           src/datatype/U2Sequence.h \
           src/datatype/U2Type.h \
           src/datatype/U2TypeIds.h \
           src/datatype/U2Variant.h \
           src/datatype/UdrRecord.h \
           src/datatype/UdrSchema.h \
           src/datatype/UdrSchemaRegistry.h \
           src/datatype/Vector3D.h \
           src/datatype/udr/RawDataUdrSchema.h \
           src/datatype/U2Msa.h \
           src/datatype/U2Mca.h \
           src/datatype/msa/MaStateCheck.h \
           src/datatype/msa/MultipleAlignment.h \
           src/datatype/msa/MultipleAlignmentInfo.h \
           src/datatype/msa/MultipleAlignmentRow.h \
           src/datatype/msa/MultipleAlignmentRowInfo.h \
           src/datatype/msa/MultipleChromatogramAlignment.h \
           src/datatype/msa/MultipleChromatogramAlignmentRow.h \
           src/datatype/msa/MultipleSequenceAlignment.h \
           src/datatype/msa/MultipleSequenceAlignmentRow.h \
           src/datatype/primers/Primer.h \
           src/datatype/primers/PrimerDimersFinder.h \
           src/datatype/primers/PrimerStatistics.h \
           src/datatype/primers/PrimerValidator.h \
           src/dbi/DbiConnection.h \
           src/dbi/DbiDocumentFormat.h \
           src/dbi/U2AbstractDbi.h \
           src/dbi/U2AssemblyDbi.h \
           src/dbi/U2AttributeDbi.h \
           src/dbi/U2CrossDatabaseReferenceDbi.h \
           src/dbi/U2Dbi.h \
           src/dbi/U2DbiPackUtils.h \
           src/dbi/U2DbiRegistry.h \
           src/dbi/U2DbiUtils.h \
           src/dbi/U2FeatureDbi.h \
           src/dbi/U2FormatCheckResult.h \
           src/dbi/U2ModDbi.h \
           src/dbi/U2MsaDbi.h \
           src/dbi/U2ObjectDbi.h \
           src/dbi/U2ObjectRelationsDbi.h \
           src/dbi/U2SequenceDbi.h \
           src/dbi/U2SqlHelpers.h \
           src/dbi/U2SQLiteTripleStore.h \
           src/dbi/U2VariantDbi.h \
           src/dbi/UdrDbi.h \
           src/globals/AppContext.h \
           src/globals/AppFileStorage.h \
           src/globals/AppGlobalObject.h \
           src/globals/AppResources.h \
           src/globals/AppSettings.h \
           src/globals/AutoAnnotationsSupport.h \
           src/globals/BaseDocumentFormats.h \
           src/globals/BundleInfo.h \
           src/globals/ClipboardController.h \
           src/globals/Counter.h \
           src/globals/CredentialsAsker.h \
           src/globals/CustomExternalTool.h \
           src/globals/DataBaseRegistry.h \
           src/globals/DataPathRegistry.h \
           src/globals/DBXRefRegistry.h \
           src/globals/ExternalToolRegistry.h \
           src/globals/FormatSettings.h \
           src/globals/ScriptEngine.h \
           src/globals/Theme.h \
           src/globals/global.h \
           src/globals/GUrl.h \
           src/globals/Identifiable.h \
           src/globals/IdRegistry.h \
           src/globals/L10n.h \
           src/globals/Log.h \
           src/globals/LogCache.h \
           src/globals/NetworkConfiguration.h \
           src/globals/Nullable.h \
           src/globals/PasswordStorage.h \
           src/globals/PluginModel.h \
           src/globals/ProjectFilterTaskRegistry.h \
           src/globals/ProjectService.h \
           src/globals/ProjectTreeControllerModeSettings.h \
           src/globals/ResourceTracker.h \
           src/globals/ScriptingToolRegistry.h \
           src/globals/ServiceModel.h \
           src/globals/ServiceTypes.h \
           src/globals/Settings.h \
           src/globals/Task.h \
           src/globals/Timer.h \
           src/globals/U2OpStatus.h \
           src/globals/U2SafePoints.h \
           src/globals/UserActionsWriter.h \
           src/globals/UserApplicationsSettings.h \
           src/globals/Version.h \
           src/gobjects/AnnotationTableObject.h \
           src/gobjects/AssemblyObject.h \
           src/gobjects/BioStruct3DObject.h \
           src/gobjects/DNAChromatogramObject.h \
           src/gobjects/DNASequenceObject.h \
           src/gobjects/GObjectRelationRoles.h \
           src/gobjects/GObjectTypes.h \
           src/gobjects/GObjectUtils.h \
           src/gobjects/MultipleAlignmentObject.h \
           src/gobjects/MultipleChromatogramAlignmentObject.h \
           src/gobjects/MultipleSequenceAlignmentObject.h \
           src/gobjects/PFMatrixObject.h \
           src/gobjects/PhyTreeObject.h \
           src/gobjects/PWMatrixObject.h \
           src/gobjects/TextObject.h \
           src/gobjects/UnloadedObject.h \
           src/gobjects/VariantTrackObject.h \
           src/io/HttpFileAdapter.h \
           src/io/InputStream.h \
           src/io/IOAdapter.h \
           src/io/IOAdapterTextStream.h \
           src/io/LocalFileAdapter.h \
           src/io/OutputStream.h \
           src/io/RingBuffer.h \
           src/io/StringAdapter.h \
           src/io/VFSAdapter.h \
           src/io/VirtualFileSystem.h \
           src/io/ZlibAdapter.h \
           src/models/BunchMimeData.h \
           src/models/DocumentImport.h \
           src/models/DocumentModel.h \
           src/models/DocumentUtils.h \
           src/models/Folder.h \
           src/models/GHints.h \
           src/models/GObject.h \
           src/models/GObjectReference.h \
           src/models/MimeDataIterator.h \
           src/models/ProjectModel.h \
           src/models/StateLockableDataModel.h \
           src/selection/AnnotationSelection.h \
           src/selection/DNASequenceSelection.h \
           src/selection/DocumentSelection.h \
           src/selection/FolderSelection.h \
           src/selection/GObjectSelection.h \
           src/selection/LRegionsSelection.h \
           src/selection/SelectionModel.h \
           src/selection/SelectionTypes.h \
           src/selection/SelectionUtils.h \
           src/selection/TextSelection.h \
           src/tasks/AbstractProjectFilterTask.h \
           src/tasks/AddDocumentTask.h \
           src/tasks/AddSequencesToAlignmentTask.h \
           src/tasks/AddObjectsToDocumentTask.h \
           src/tasks/BackgroundTaskRunner.h \
           src/tasks/CleanupFileStorageTask.h \
           src/tasks/CloneObjectTask.h \
           src/tasks/CmdlineInOutTaskRunner.h \
           src/tasks/CmdlineTaskRunner.h \
           src/tasks/ConsoleShutdownTask.h \
           src/tasks/CopyDataTask.h \
           src/tasks/CopyDocumentTask.h \
           src/tasks/CopyFileTask.h \
           src/tasks/CreateAnnotationTask.h \
           src/tasks/DeleteObjectsTask.h \
           src/tasks/DocumentProviderTask.h \
           src/tasks/ExportSequencesTask.h \
           src/tasks/ExternalToolRunTask.h \
           src/tasks/ExtractAnnotatedRegionTask.h \
           src/tasks/FailTask.h \
           src/tasks/LoadDocumentTask.h \
           src/tasks/LoadRemoteDocumentTask.h \
           src/tasks/ModifySequenceObjectTask.h \
           src/tasks/MultiTask.h \
           src/tasks/RemoveAnnotationsTask.h \
           src/tasks/RemoveDocumentTask.h \
           src/tasks/ReverseSequenceTask.h \
           src/tasks/SaveDocumentStreamingTask.h \
           src/tasks/SaveDocumentTask.h \
           src/tasks/ScriptTask.h \
           src/tasks/SequenceDbiWalkerTask.h \
           src/tasks/SequenceWalkerTask.h \
           src/tasks/TaskSignalMapper.h \
           src/tasks/TaskStarter.h \
           src/tasks/TLSTask.h \
           src/tasks/TmpDirChecker.h \
           src/tasks/shared_db/ImportDirToDatabaseTask.h \
           src/tasks/shared_db/ImportDocumentToDatabaseTask.h \
           src/tasks/shared_db/ImportFileToDatabaseTask.h \
           src/tasks/shared_db/ImportObjectToDatabaseTask.h \
           src/tasks/shared_db/ImportToDatabaseTask.h \
           src/util/AnnotationCreationPattern.h \
           src/util/AssemblyImporter.h \
           src/util/ChromatogramUtils.h \
           src/util/DatatypeSerializeUtils.h \
           src/util/FileAndDirectoryUtils.h \
           src/util/FilesIterator.h \
           src/util/FileFilters.h \
           src/util/FileStorageUtils.h \
           src/util/FormatUtils.h \
           src/util/Formatters.h \
           src/util/GAutoDeleteList.h \
           src/util/GenbankFeatures.h \
           src/util/GUrlUtils.h \
           src/util/ImportToDatabaseOptions.h \
           src/util/IOAdapterUtils.h \
           src/util/MaIterator.h \
           src/util/MaModificationInfo.h \
           src/util/McaDbiUtils.h \
           src/util/MsaDbiUtils.h \
           src/util/McaRowInnerData.h \
           src/util/MsaRowUtils.h \
           src/util/MSAUtils.h \
           src/util/MultipleChromatogramAlignmentExporter.h \
           src/util/MultipleChromatogramAlignmentImporter.h \
           src/util/MultipleSequenceAlignmentExporter.h \
           src/util/MultipleSequenceAlignmentImporter.h \
           src/util/MultipleSequenceAlignmentWalker.h \
           src/util/PMatrixSerializeUtils.h \
           src/util/QObjectScopedPointer.h \
           src/util/QVariantUtils.h \
           src/util/SequenceUtils.h \
           src/util/SnpeffDictionary.h \
           src/util/StrPackUtils.h \
           src/util/SyncHttp.h \
           src/util/TextUtils.h \
           src/util/TaskWatchdog.h \
           src/util/U1AnnotationUtils.h \
           src/util/U2AlphabetUtils.h \
           src/util/U2AssemblyReadIterator.h \
           src/util/U2AssemblyUtils.h \
           src/util/U2AttributeUtils.h \
           src/util/U2Bits.h \
           src/util/U2DbiUpgrader.h \
           src/util/U2FeatureUtils.h \
           src/util/U2ObjectTypeUtils.h \
           src/util/U2OpStatusUtils.h \
           src/util/U2SequenceUtils.h \
           src/util/U2VariationUtils.h \
           src/util/VariationPropertiesUtils.h

SOURCES += src/cmdline/CMDLineCoreOptions.cpp \
           src/cmdline/CMDLineRegistry.cpp \
           src/cmdline/CMDLineUtils.cpp \
           src/datatype/Annotation.cpp \
           src/datatype/AnnotationData.cpp \
           src/datatype/AnnotationGroup.cpp \
           src/datatype/AnnotationModification.cpp \
           src/datatype/AnnotationSettings.cpp \
           src/datatype/AnnotationTableObjectConstraints.cpp \
           src/datatype/BaseAlphabets.cpp \
           src/datatype/BaseTranslations.cpp \
           src/datatype/BioStruct3D.cpp \
           src/datatype/DNAAlphabet.cpp \
           src/datatype/DNAAlphabetRegistryImpl.cpp \
           src/datatype/DNAInfo.cpp \
           src/datatype/DNAQuality.cpp \
           src/datatype/DNASequence.cpp \
           src/datatype/DNASequenceUtils.cpp \
           src/datatype/DNATranslation.cpp \
           src/datatype/DNATranslationImpl.cpp \
           src/datatype/FeatureColors.cpp \
           src/datatype/Matrix44.cpp \
           src/datatype/PFMatrix.cpp \
           src/datatype/PhyTree.cpp \
           src/datatype/PWMatrix.cpp \
           src/datatype/SMatrix.cpp \
           src/datatype/U2CoreAttributes.cpp \
           src/datatype/U2FeatureType.cpp \
           src/datatype/U2Mod.cpp \
           src/datatype/U2Qualifier.cpp \
           src/datatype/U2Region.cpp \
           src/datatype/U2Type.cpp \
           src/datatype/U2Variant.cpp \
           src/datatype/UdrRecord.cpp \
           src/datatype/UdrSchema.cpp \
           src/datatype/UdrSchemaRegistry.cpp \
           src/datatype/Vector3D.cpp \
           src/datatype/U2Mca.cpp \
           src/datatype/U2Msa.cpp \
           src/datatype/DNAChromatogram.cpp \
           src/datatype/msa/MaStateCheck.cpp \
           src/datatype/msa/MultipleAlignment.cpp \
           src/datatype/msa/MultipleAlignmentInfo.cpp \
           src/datatype/msa/MultipleAlignmentRow.cpp \
           src/datatype/msa/MultipleAlignmentRowInfo.cpp \
           src/datatype/msa/MultipleChromatogramAlignmentRow.cpp \
           src/datatype/msa/MultipleChromatogramAlignment.cpp \
           src/datatype/msa/MultipleSequenceAlignment.cpp \
           src/datatype/msa/MultipleSequenceAlignmentRow.cpp \
           src/datatype/primers/Primer.cpp \
           src/datatype/primers/PrimerDimersFinder.cpp \
           src/datatype/primers/PrimerStatistics.cpp \
           src/datatype/primers/PrimerValidator.cpp \
           src/datatype/udr/RawDataUdrSchema.cpp \
           src/dbi/DbiConnection.cpp \
           src/dbi/DbiDocumentFormat.cpp \
           src/dbi/U2Dbi.cpp \
           src/dbi/U2DbiPackUtils.cpp \
           src/dbi/U2DbiRegistry.cpp \
           src/dbi/U2DbiUtils.cpp \
           src/dbi/U2ObjectDbi.cpp \
           src/dbi/U2ObjectRelationsDbi.cpp \
           src/dbi/U2SqlHelpers.cpp \
           src/dbi/U2SQLiteTripleStore.cpp \
           src/dbi/UdrDbi.cpp \
           src/globals/AppContext.cpp \
           src/globals/AppFileStorage.cpp \
           src/globals/AppGlobalObject.cpp \
           src/globals/AppResources.cpp \
           src/globals/AutoAnnotationsSupport.cpp \
           src/globals/BaseDocumentFormats.cpp \
           src/globals/BundleInfo.cpp \
           src/globals/ClipboardController.cpp \
           src/globals/Counter.cpp \
           src/globals/CredentialsAsker.cpp \
           src/globals/CustomExternalTool.cpp \
           src/globals/DataBaseRegistry.cpp \
           src/globals/DataPathRegistry.cpp \
           src/globals/DBXRefRegistry.cpp \
           src/globals/ExternalToolRegistry.cpp \
           src/globals/FormatSettings.cpp \
           src/globals/GUrl.cpp \
           src/globals/Log.cpp \
           src/globals/LogCache.cpp \
           src/globals/NetworkConfiguration.cpp \
           src/globals/PasswordStorage.cpp \
           src/globals/PluginModel.cpp \
           src/globals/ProjectFilterTaskRegistry.cpp \
           src/globals/ProjectService.cpp \
           src/globals/ProjectTreeControllerModeSettings.cpp \
           src/globals/ResourceTracker.cpp \
           src/globals/ScriptingToolRegistry.cpp \
           src/globals/ServiceModel.cpp \
           src/globals/Task.cpp \
           src/globals/Timer.cpp \
           src/globals/U2SafePoints.cpp \
           src/globals/UserActionsWriter.cpp \
           src/globals/UserApplicationsSettings.cpp \
           src/globals/Version.cpp \
           src/gobjects/AnnotationTableObject.cpp \
           src/gobjects/AssemblyObject.cpp \
           src/gobjects/BioStruct3DObject.cpp \
           src/gobjects/DNAChromatogramObject.cpp \
           src/gobjects/DNASequenceObject.cpp \
           src/gobjects/GObjectRelationRoles.cpp \
           src/gobjects/GObjectTypes.cpp \
           src/gobjects/GObjectUtils.cpp \
           src/gobjects/MultipleAlignmentObject.cpp \
           src/gobjects/MultipleChromatogramAlignmentObject.cpp \
           src/gobjects/MultipleSequenceAlignmentObject.cpp \
           src/gobjects/PFMatrixObject.cpp \
           src/gobjects/PhyTreeObject.cpp \
           src/gobjects/PWMatrixObject.cpp \
           src/gobjects/TextObject.cpp \
           src/gobjects/UnloadedObject.cpp \
           src/gobjects/VariantTrackObject.cpp \
           src/io/HttpFileAdapter.cpp \
           src/io/IOAdapter.cpp \
           src/io/IOAdapterTextStream.cpp \
           src/io/LocalFileAdapter.cpp \
           src/io/StringAdapter.cpp \
           src/io/VFSAdapter.cpp \
           src/io/VirtualFileSystem.cpp \
           src/io/ZlibAdapter.cpp \
           src/models/BunchMimeData.cpp \
           src/models/DocumentImport.cpp \
           src/models/DocumentModel.cpp \
           src/models/DocumentUtils.cpp \
           src/models/Folder.cpp \
           src/models/GHints.cpp \
           src/models/GObject.cpp \
           src/models/GObjectReference.cpp \
           src/models/MimeDataIterator.cpp \
           src/models/ProjectModel.cpp \
           src/models/StateLockableDataModel.cpp \
           src/selection/AnnotationSelection.cpp \
           src/selection/DocumentSelection.cpp \
           src/selection/FolderSelection.cpp \
           src/selection/GObjectSelection.cpp \
           src/selection/LRegionsSelection.cpp \
           src/selection/SelectionModel.cpp \
           src/selection/SelectionTypes.cpp \
           src/selection/SelectionUtils.cpp \
           src/tasks/AbstractProjectFilterTask.cpp \
           src/tasks/AddDocumentTask.cpp \
           src/tasks/AddSequencesToAlignmentTask.cpp \
           src/tasks/AddObjectsToDocumentTask.cpp \
           src/tasks/BackgroundTaskRunner.cpp \
           src/tasks/CleanupFileStorageTask.cpp \
           src/tasks/CloneObjectTask.cpp \
           src/tasks/CmdlineInOutTaskRunner.cpp \
           src/tasks/CmdlineTaskRunner.cpp \
           src/tasks/ConsoleShutdownTask.cpp \
           src/tasks/CopyDataTask.cpp \
           src/tasks/CopyDocumentTask.cpp \
           src/tasks/CopyFileTask.cpp \
           src/tasks/CreateAnnotationTask.cpp \
           src/tasks/DeleteObjectsTask.cpp \
           src/tasks/DocumentProviderTask.cpp \
           src/tasks/ExportSequencesTask.cpp \
           src/tasks/ExternalToolRunTask.cpp \
           src/tasks/ExtractAnnotatedRegionTask.cpp \
           src/tasks/LoadDocumentTask.cpp \
           src/tasks/LoadRemoteDocumentTask.cpp \
           src/tasks/ModifySequenceObjectTask.cpp \
           src/tasks/MultiTask.cpp \
           src/tasks/RemoveAnnotationsTask.cpp \
           src/tasks/RemoveDocumentTask.cpp \
           src/tasks/ReverseSequenceTask.cpp \
           src/tasks/SaveDocumentStreamingTask.cpp \
           src/tasks/SaveDocumentTask.cpp \
           src/tasks/ScriptTask.cpp \
           src/tasks/SequenceDbiWalkerTask.cpp \
           src/tasks/SequenceWalkerTask.cpp \
           src/tasks/TaskSignalMapper.cpp \
           src/tasks/TaskStarter.cpp \
           src/tasks/TLSTask.cpp \
           src/tasks/TmpDirChecker.cpp \
           src/tasks/shared_db/ImportDirToDatabaseTask.cpp \
           src/tasks/shared_db/ImportDocumentToDatabaseTask.cpp \
           src/tasks/shared_db/ImportFileToDatabaseTask.cpp \
           src/tasks/shared_db/ImportObjectToDatabaseTask.cpp \
           src/tasks/shared_db/ImportToDatabaseTask.cpp \
           src/util/AnnotationCreationPattern.cpp \
           src/util/AssemblyImporter.cpp \
           src/util/ChromatogramUtils.cpp \
           src/util/DatatypeSerializeUtils.cpp \
           src/util/FileAndDirectoryUtils.cpp \
           src/util/FilesIterator.cpp \
           src/util/FileFilters.cpp \
           src/util/FileStorageUtils.cpp \
           src/util/FormatUtils.cpp \
           src/util/Formatters.cpp \
           src/util/GAutoDeleteList.cpp \
           src/util/GenbankFeatures.cpp \
           src/util/GUrlUtils.cpp \
           src/util/ImportToDatabaseOptions.cpp \
           src/util/IOAdapterUtils.cpp \
           src/util/MaIterator.cpp \
           src/util/MaModificationInfo.cpp \
           src/util/McaDbiUtils.cpp \
           src/util/McaRowInnerData.cpp \
           src/util/MsaDbiUtils.cpp \
           src/util/MsaRowUtils.cpp \
           src/util/MSAUtils.cpp \
           src/util/MultipleChromatogramAlignmentExporter.cpp \
           src/util/MultipleChromatogramAlignmentImporter.cpp \
           src/util/MultipleSequenceAlignmentExporter.cpp \
           src/util/MultipleSequenceAlignmentImporter.cpp \
           src/util/MultipleSequenceAlignmentWalker.cpp \
           src/util/QVariantUtils.cpp \
           src/util/SequenceUtils.cpp \
           src/util/SnpeffDictionary.cpp \
           src/util/StrPackUtils.cpp \
           src/util/SyncHttp.cpp \
           src/util/TextUtils.cpp \
           src/util/TaskWatchdog.cpp \
           src/util/U1AnnotationUtils.cpp \
           src/util/U2AlphabetUtils.cpp \
           src/util/U2AssemblyReadIterator.cpp \
           src/util/U2AssemblyUtils.cpp \
           src/util/U2AttributeUtils.cpp \
           src/util/U2Bits.cpp \
           src/util/U2DbiUpgrader.cpp \
           src/util/U2FeatureUtils.cpp \
           src/util/U2ObjectTypeUtils.cpp \
           src/util/U2SequenceUtils.cpp \
           src/util/U2VariationUtils.cpp

TRANSLATIONS += transl/russian.ts
