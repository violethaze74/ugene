
namespace U2 {

const QString FindPatternWidgetFactory::GROUP_ID = "OP_FIND_PATTERN";
const QString FindPatternWidgetFactory::GROUP_ICON_STR = ":core/images/find_dialog.png";
const QString FindPatternWidgetFactory::GROUP_DOC_PAGE = "18222978";

FindPatternWidgetFactory::FindPatternWidgetFactory() {
    objectViewOfWidget = ObjViewType_SequenceView;
}

QWidget * FindPatternWidgetFactory::createWidget(GObjectView* objView) {
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    AnnotatedDNAView* annotatedDnaView = qobject_cast<AnnotatedDNAView*>(objView);
    SAFE_POINT(NULL != annotatedDnaView,
        QString("Internal error: unable to cast object view to AnnotatedDNAView for group '%1'.").arg(GROUP_ID),
        NULL);

    FindPatternWidget* widget = new FindPatternWidget(annotatedDnaView);
    widget->setObjectName("FindPatternWidget");
    return widget;
}

OPGroupParameters FindPatternWidgetFactory::getOPGroupParameters() {
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), QObject::tr("Search in Sequence"), GROUP_DOC_PAGE);
}

const QString & FindPatternWidgetFactory::getGroupId() {
    return GROUP_ID;
}

} // namespace U2
