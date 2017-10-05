/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_EDITOR_TASKS_H_
#define _U2_MSA_EDITOR_TASKS_H_

#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/DocumentProviderTask.h>
namespace U2 {

class MultipleAlignmentObject;
class UnloadedObject;
class MaEditor;
class MaEditorFactory;
class MSAEditor;

/*!
 * \brief The OpenMaEditorTask class
 */
class OpenMaEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenMaEditorTask(MultipleAlignmentObject* obj, GObjectViewFactoryId fid, GObjectType type);
    OpenMaEditorTask(UnloadedObject* obj, GObjectViewFactoryId fid, GObjectType type);
    OpenMaEditorTask(Document* doc, GObjectViewFactoryId fid, GObjectType type);

    virtual void open();

    static void updateTitle(MSAEditor* msaEd);

    virtual MaEditor* getEditor(const QString& viewName, GObject* obj) = 0;

protected:
    GObjectType                         type;
    QPointer<MultipleAlignmentObject>   maObject;
    GObjectReference                    unloadedReference;
};

/*!
 * \brief The OpenMsaEditorTaskOpenMsaEditorTask class
 */
class OpenMsaEditorTask : public OpenMaEditorTask {
    Q_OBJECT
public:
    OpenMsaEditorTask(MultipleAlignmentObject* obj);
    OpenMsaEditorTask(UnloadedObject* obj);
    OpenMsaEditorTask(Document* doc);

    MaEditor* getEditor(const QString &viewName, GObject *obj);
};

/*!
 * \brief The OpenMcaEditorTask class
 */
class OpenMcaEditorTask : public OpenMaEditorTask {
    Q_OBJECT
public:
    OpenMcaEditorTask(MultipleAlignmentObject* obj);
    OpenMcaEditorTask(UnloadedObject* obj);
    OpenMcaEditorTask(Document* doc);

    MaEditor* getEditor(const QString &viewName, GObject *obj);
};

class OpenSavedMaEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedMaEditorTask(GObjectType type, MaEditorFactory* factory,
                          const QString& viewName, const QVariantMap& stateData);
    virtual void open();

    static void updateRanges(const QVariantMap& stateData, MaEditor* ctx);
private:
    GObjectType         type;
    MaEditorFactory* factory;
};


class UpdateMSAEditorTask : public ObjectViewTask {
public:
    UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData);

    virtual void update();
};

class ExportMaConsensusTaskSettings {
public:
    ExportMaConsensusTaskSettings();

    bool                keepGaps;
    MaEditor*           ma;
    QString             url;
    DocumentFormatId    format;
    QString             name;
};

class ExtractConsensusTask : public Task {
    Q_OBJECT
public:
    ExtractConsensusTask( bool keepGaps, MaEditor* ma);
    void run();
    const QByteArray& getExtractedConsensus() const;
private:
    bool        keepGaps;
    MaEditor*   ma;
    QByteArray  filteredConsensus;
};

class ExportMaConsensusTask : public DocumentProviderTask {
    Q_OBJECT
public:
    ExportMaConsensusTask(const ExportMaConsensusTaskSettings& s);

    void prepare();
protected:
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    Document* createDocument();

    ExportMaConsensusTaskSettings   settings;
    ExtractConsensusTask*           extractConsensus;
    QByteArray                      filteredConsensus;
};

} // namespace

#endif
