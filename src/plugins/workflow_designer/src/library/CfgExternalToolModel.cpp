/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "CfgExternalToolModel.h"
#include "../WorkflowEditorDelegates.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModel
//////////////////////////////////////////////////////////////////////////

CfgExternalToolItem::CfgExternalToolItem()  {
    dfr = AppContext::getDocumentFormatRegistry();
    dtr = Workflow::WorkflowEnv::getDataTypeRegistry();

    delegateForTypes = NULL;
    delegateForFormats = NULL;
    itemData.type = BaseTypes::DNA_SEQUENCE_TYPE()->getId();
    itemData.format = BaseDocumentFormats::FASTA;
}

CfgExternalToolItem::~CfgExternalToolItem() {
    delete delegateForTypes;
    delete delegateForFormats;
}

QString CfgExternalToolItem::getDataType() const {
    return itemData.type;
}

void CfgExternalToolItem::setDataType(const QString& id) {
    itemData.type = id;
}

QString CfgExternalToolItem::getName() const {
    return itemData.attrName;
}

void CfgExternalToolItem::setName(const QString &_name) {
    itemData.attrName = _name;
}

QString CfgExternalToolItem::getFormat() const {
    return itemData.format;
}

void CfgExternalToolItem::setFormat(const QString & f) {
    itemData.format = f;
}

QString CfgExternalToolItem::getDescription() const {
    return itemData.description;
}

void CfgExternalToolItem::setDescription(const QString & _descr) {
    itemData.description = _descr;
}

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModel
//////////////////////////////////////////////////////////////////////////

CfgExternalToolModel::CfgExternalToolModel(CfgExternalToolModel::ModelType _modelType, QObject *_obj)
    : QAbstractTableModel(_obj),
      isInput(Input == _modelType)
{
    init();
}

int CfgExternalToolModel::rowCount(const QModelIndex & /*index*/) const{
    return items.size();
}

int CfgExternalToolModel::columnCount(const QModelIndex & /*index*/) const {
    return COLUMNS_COUNT;
}

Qt::ItemFlags CfgExternalToolModel::flags(const QModelIndex & /*index*/) const{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

CfgExternalToolItem* CfgExternalToolModel::getItem(const QModelIndex &index) const {
    return items.at(index.row());
}

QList<CfgExternalToolItem*> CfgExternalToolModel::getItems() const {
    return items;
}

QVariant CfgExternalToolModel::data(const QModelIndex &index, int role) const {
    CfgExternalToolItem *item = getItem(index);
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole: // fallthrough
    case Qt::ToolTipRole:
        switch (col) {
        case COLUMN_NAME:
            return role == Qt::ToolTipRole ? tr("Command line parameter name") : item->getName();
        case COLUMN_DATA_TYPE:
            return item->delegateForTypes->getDisplayValue(item->getDataType());
        case COLUMN_FORMAT:
            return item->delegateForFormats->getDisplayValue(item->getFormat());
        case COLUMN_DESCRIPTION:
            return item->getDescription();
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    case DelegateRole:
        switch (col) {
        case COLUMN_DATA_TYPE:
            return qVariantFromValue<PropertyDelegate*>(item->delegateForTypes);
        case COLUMN_FORMAT:
            return qVariantFromValue<PropertyDelegate*>(item->delegateForFormats);
        default:
            return QVariant();
        }
    case Qt::EditRole: // fallthrough
    case ConfigurationEditor::ItemValueRole:
        switch (col) {
        case COLUMN_NAME:
            return item->getName();
        case COLUMN_DATA_TYPE:
            return item->getDataType();
        case COLUMN_FORMAT:
            return item->getFormat();
        case COLUMN_DESCRIPTION:
            return item->getDescription();
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    default:
        return QVariant();
    }
}

void CfgExternalToolModel::createFormatDelegate(const QString &newType, CfgExternalToolItem *item) {
    PropertyDelegate *delegate;
    QString format;
    if (newType == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(seqFormatsW);
        format = seqFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(msaFormatsW);
        format = msaFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(annFormatsW);
        format = annFormatsW.values().first().toString();
    }
    else if (newType == SEQ_WITH_ANNS){
        delegate = new ComboBoxDelegate(annFormatsW);
        format = annFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::STRING_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(textFormat);
        format = textFormat.values().first().toString();
    }
    else{
        return;
    }
    item->setFormat(format);
    item->delegateForFormats = delegate;
}

bool CfgExternalToolModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int col = index.column();
    CfgExternalToolItem * item = getItem(index);
    switch (role) {
    case Qt::EditRole: // fallthrough
    case ConfigurationEditor::ItemValueRole:
        switch (col) {
        case COLUMN_NAME:
            if (item->getName() != value.toString()) {
                item->setName(value.toString());
            }
            break;
        case COLUMN_DATA_TYPE: {
            QString newType = value.toString();
            if (item->getDataType() != newType) {
                if (!newType.isEmpty()) {
                    item->setDataType(newType);
                    createFormatDelegate(newType, item);
                }
            }
            break;
        }
        case COLUMN_FORMAT:
            if (item->getFormat() != value.toString() && !value.toString().isEmpty())  {
                item->setFormat(value.toString());
            }
            break;
        case COLUMN_DESCRIPTION:
            if (item->getDescription() != value.toString()) {
                item->setDescription(value.toString());
            }
            break;
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
        }
        emit dataChanged(index, index);
        break;
    default:
        ; // do nothing
    }
    return true;
}

QVariant CfgExternalToolModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case COLUMN_NAME:
            return tr("Name");
        case COLUMN_DATA_TYPE:
            return tr("Type");
        case COLUMN_FORMAT:
            if (isInput) {
                return tr("Read as");
            } else {
                return tr("Write as");
            }
        case COLUMN_DESCRIPTION:
            return tr("Description");
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    }
    if (role == Qt::ToolTipRole) {
        if (section == COLUMN_NAME) {
            return tr("Command line parameter name");
        }
    }
    return QVariant();
}

bool CfgExternalToolModel::insertRows(int /*row*/, int /*count*/, const QModelIndex &parent) {
    beginInsertRows(parent, items.size(), items.size());
    CfgExternalToolItem *newItem = new CfgExternalToolItem();
    newItem->delegateForTypes = new ComboBoxDelegate(types);
    newItem->delegateForFormats = new ComboBoxDelegate(seqFormatsW);
    items.append(newItem);
    endInsertRows();
    return true;
}

bool CfgExternalToolModel::removeRows(int row, int count, const QModelIndex &parent) {
    CHECK(0 <= row && row < items.size(), false);
    CHECK(0 <= row + count - 1 && row + count - 1 < items.size(), false);
    CHECK(0 < count, false);

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row + count - 1; i >= row; --i) {
        delete items.takeAt(i);
    }
    endRemoveRows();
    return true;
}

void CfgExternalToolModel::init() {
    initTypes();
    initFormats();
}

void CfgExternalToolModel::initFormats() {
    QList<DocumentFormatId> ids = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();

    DocumentFormatConstraints commonConstraints;
    commonConstraints.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    commonConstraints.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    commonConstraints.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);

    DocumentFormatConstraints seqWrite(commonConstraints);
    seqWrite.supportedObjectTypes += GObjectTypes::SEQUENCE;

    DocumentFormatConstraints seqRead(commonConstraints);
    seqRead.supportedObjectTypes += GObjectTypes::SEQUENCE;

    DocumentFormatConstraints msaWrite(commonConstraints);
    msaWrite.supportedObjectTypes += GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT;

    DocumentFormatConstraints msaRead(commonConstraints);
    msaRead.supportedObjectTypes += GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT;

    DocumentFormatConstraints annWrite(commonConstraints);
    annWrite.supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;

    DocumentFormatConstraints annRead(commonConstraints);
    annRead.supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;

    foreach(const DocumentFormatId& id, ids) {
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(id);

        if (df->checkConstraints(seqWrite)) {
            seqFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(seqRead)) {
            seqFormatsR[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(msaWrite)) {
            msaFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(msaRead)) {
            msaFormatsR[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(annWrite)) {
            annFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(annRead)) {
            annFormatsR[df->getFormatName()] = df->getFormatId();
        }
    }

    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_TEXT);
    if (isInput) {
        textFormat[tr("String value")] = DataConfig::STRING_VALUE;
    } else {
        textFormat[tr("Output file url")] = DataConfig::OUTPUT_FILE_URL;
    }
    textFormat[tr("Text file")] = df->getFormatId();
}

void CfgExternalToolModel::initTypes() {
    DataTypePtr ptr = BaseTypes::DNA_SEQUENCE_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::ANNOTATION_TABLE_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::STRING_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    types["Sequence with annotations"] = SEQ_WITH_ANNS;
}

//////////////////////////////////////////////////////////////////////////
/// AttributeItem
//////////////////////////////////////////////////////////////////////////

const QString &AttributeItem::getName() const {
    return name;
}

void AttributeItem::setName(const QString& _name) {
    name = _name;
}

const QString &AttributeItem::getDataType() const {
    return type;
}

void AttributeItem::setDataType(const QString &_type) {
    type = _type;
}

const QString &AttributeItem::getDefaultValue() const {
    return defaultValue;
}

void AttributeItem::setDefaultValue(const QString &_defaultValue) {
    defaultValue = _defaultValue;
}

const QString &AttributeItem::getDescription() const {
    return description;
}

void AttributeItem::setDescription(const QString &_description) {
    description = _description;
}

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModelAttributes
//////////////////////////////////////////////////////////////////////////

CfgExternalToolModelAttributes::CfgExternalToolModelAttributes() {
    types["URL"] = "URL";
    types["String"] = "String";
    types["Number"] = "Number";
    types["Boolean"] = "Boolean";
    delegate = new ComboBoxDelegate(types);
}

CfgExternalToolModelAttributes::~CfgExternalToolModelAttributes() {
    foreach(AttributeItem* item, items) {
        delete item;
    }
}
int CfgExternalToolModelAttributes::rowCount(const QModelIndex & /*index*/) const{
    return items.size();
}

int CfgExternalToolModelAttributes::columnCount(const QModelIndex & /*index*/) const {
    return COLUMNS_COUNT;
}

Qt::ItemFlags CfgExternalToolModelAttributes::flags(const QModelIndex & /*index*/) const{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

AttributeItem* CfgExternalToolModelAttributes::getItem(const QModelIndex &index) const {
    return items.at(index.row());
}

QList<AttributeItem*> CfgExternalToolModelAttributes::getItems() const {
    return items;
}

QVariant CfgExternalToolModelAttributes::data(const QModelIndex &index, int role) const {
    AttributeItem *item = getItem(index);
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole: // fallthrough
    case Qt::ToolTipRole:
        switch (col) {
        case COLUMN_NAME:
            return item->getName();
        case COLUMN_DATA_TYPE:
            return delegate->getDisplayValue(item->getDataType());
        case COLUMN_DEFAULT_VALUE:
            return item->getDefaultValue();
        case COLUMN_DESCRIPTION:
            return item->getDescription();
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    case DelegateRole:
        if (COLUMN_DATA_TYPE == col) {
            return qVariantFromValue<PropertyDelegate*>(delegate);
        } else {
            return QVariant();
        }
    case Qt::EditRole: // fallthrough
    case ConfigurationEditor::ItemValueRole:
        switch (col) {
        case COLUMN_NAME:
            return item->getName();
        case COLUMN_DATA_TYPE:
            return item->getDataType();
        case COLUMN_DEFAULT_VALUE:
            return item->getDefaultValue();
        case COLUMN_DESCRIPTION:
            return item->getDescription();
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    default:
        return QVariant();
    }
}

bool CfgExternalToolModelAttributes::setData(const QModelIndex &index, const QVariant &value, int role) {
    int col = index.column();
    AttributeItem * item = getItem(index);
    switch (role) {
    case Qt::EditRole: // fallthrough
    case ConfigurationEditor::ItemValueRole:
        switch (col) {
        case COLUMN_NAME:
            if (item->getName() != value.toString()) {
                item->setName(value.toString());
            }
            break;
        case COLUMN_DATA_TYPE: {
            QString newType = value.toString();
            if (item->getDataType() != newType) {
                if (!newType.isEmpty()) {
                    item->setDataType(newType);
                }
            }
            break;
        }
        case COLUMN_DEFAULT_VALUE: {
            if (item->getDefaultValue() != value.toString()) {
                item->setDefaultValue(value.toString());
            }
            break;
        }
        case COLUMN_DESCRIPTION:
            if (item->getDescription() != value.toString()) {
                item->setDescription(value.toString());
            }
            break;
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
        }

        emit dataChanged(index, index);
        break;
    default:
        ; // do nothing
    }
    return true;
}

QVariant CfgExternalToolModelAttributes::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case COLUMN_NAME:
            return tr("Name");
        case COLUMN_DATA_TYPE:
            return tr("Type");
        case COLUMN_DEFAULT_VALUE:
            return tr("Default value");
        case COLUMN_DESCRIPTION:
            return tr("Description");
        default:
            // do nothing, inaccessible code
            Q_ASSERT(false);
            return QVariant();
        }
    }
    return QVariant();
}

bool CfgExternalToolModelAttributes::insertRows(int /*row*/, int /*count*/, const QModelIndex & parent)  {
    beginInsertRows(parent, items.size(), items.size());
    AttributeItem *newItem = new AttributeItem();
    newItem->setDataType("String");
    items.append(newItem);
    endInsertRows();
    return true;
}

bool CfgExternalToolModelAttributes::removeRows(int row, int count, const QModelIndex & parent) {
    CHECK(0 <= row && row < items.size(), false);
    CHECK(0 <= row + count - 1 && row + count - 1 < items.size(), false);
    CHECK(0 < count, false);

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row + count - 1; i >= row; --i) {
        delete items.takeAt(i);
    }
    endRemoveRows();
    return true;
}

} // U2
