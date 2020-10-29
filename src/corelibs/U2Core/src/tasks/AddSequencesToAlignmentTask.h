/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
#define _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_

#include <QPointer>

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/Task.h>
#include <U2Core/U2Type.h>

namespace U2 {

class StateLock;
class LoadDocumentTask;
class DNASequence;
class U2MsaDbi;
class U2UseCommonUserModStep;

class U2CORE_EXPORT AddSequenceObjectsToAlignmentTask : public Task {
    Q_OBJECT
public:
    /**
     * Adds sequences to the alignment starting from the given insertRowIndex: making the first inserted row to have row index = insertRowIndex.
     * For example insertRowIndex = 0 pre-appends sequences to the alignment.
     * If insertRowIndex < 0 or insertRowIndex >= row count - appends sequences to the end of the alignment rows.
     */
    AddSequenceObjectsToAlignmentTask(MultipleSequenceAlignmentObject *obj,
                                      const QList<DNASequence> &seqList,
                                      int insertRowIndex = -1,
                                      bool recheckNewSequenceAlphabetOnMismatch = false);

    virtual void prepare();
    virtual void run();
    ReportResult report();

    const MaModificationInfo &getMaModificationInfo() const {
        return mi;
    }

protected:
    void processObjectsAndSetResultingAlphabet();

    QList<DNASequence> seqList;
    int insertRowIndex;
    QPointer<MultipleSequenceAlignmentObject> maObj;

protected:
    void releaseLock();

private:
    StateLock *stateLock;
    const DNAAlphabet *msaAlphabet;
    QStringList errorList;
    U2MsaDbi *dbi;
    U2EntityRef entityRef;
    U2UseCommonUserModStep *modStep;
    MaModificationInfo mi;

    /*
     * If re-check alphabet is true and the alphabet of the new sequence is not the same as the alphabet of the alignment
     * the task will re-test the sequence data if it fits into the alignment alphabet first and fall-back to the seqeuence alphabet only if it does not.
     *
     * Example: paste symbol 'T' to amino alignment: 'T' is detected as Nucleic while it is also valid for Amino!
     */
    bool recheckNewSequenceAlphabetOnMismatch;

    static const int maxErrorListSize;
    /** Returns the max length of the rows including trailing gaps */
    qint64 createRows(QList<U2MsaRow> &rows);
    void addRows(QList<U2MsaRow> &rows, qint64 maxLength);
    void updateAlphabet();
    void setupError();
};

class U2CORE_EXPORT AddSequencesFromFilesToAlignmentTask : public AddSequenceObjectsToAlignmentTask {
    Q_OBJECT
public:
    AddSequencesFromFilesToAlignmentTask(MultipleSequenceAlignmentObject *obj, const QStringList &urls, int insertRowIndex);

    virtual void prepare();
    QList<Task *> onSubTaskFinished(Task *subTask);
private slots:
    void sl_onCancel();

private:
    QStringList urlList;
    LoadDocumentTask *loadTask;
};

class U2CORE_EXPORT AddSequencesFromDocumentsToAlignmentTask : public AddSequenceObjectsToAlignmentTask {
    Q_OBJECT
public:
    AddSequencesFromDocumentsToAlignmentTask(MultipleSequenceAlignmentObject *obj, const QList<Document *> &docs, int insertRowIndex, bool recheckNewSequenceAlphabets);

    virtual void prepare();

private:
    QList<Document *> docs;
};

}    // namespace U2

#endif    //_U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
