#ifndef KALIGNSUBTASK_H
#define KALIGNSUBTASK_H

#include <U2Core/Task.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>

namespace U2 {
class AbstractAlignmentTaskFactory;
class PairwiseAlignmentTaskSettings;
namespace Workflow {

class KAlignSubTask : public Task {
    Q_OBJECT
public:
    KAlignSubTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage);
    void prepare();
    void run();

    const SharedDbiDataHandler getRead() const;
    qint64 getMaxRegionSize() const;
    U2Region getCoreRegion() const;

    static PairwiseAlignmentTaskSettings * createSettings(DbiDataStorage *storage, const SharedDbiDataHandler &msa, U2OpStatus &os);
    static AbstractAlignmentTaskFactory * getAbstractAlignmentTaskFactory(const QString &algoId, const QString &implId, U2OpStatus &os);

private:
    void createAlignment();
    QList<U2Region> getRegions(const QList<U2MsaGap> &gaps, qint64 rowLength) const;
    void calculateCoreRegion(const QList<U2Region> &regions);
    void extendCoreRegion(const QList<U2Region> &regions);

private:
    const SharedDbiDataHandler reference;
    const SharedDbiDataHandler read;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;

    qint64 maxRegionSize;
    U2Region coreRegion;

    static const qint64 MAX_GAP_SIZE;
    static const qint64 EXTENSION_COEF;
};

} // namespace Workflow
} // namespace U2

#endif // KALIGNSUBTASK_H
