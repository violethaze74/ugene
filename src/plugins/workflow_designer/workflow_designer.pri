# include (workflow_designer.pri)

PLUGIN_ID=workflow_designer
PLUGIN_NAME=Workflow Designer
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

QT += scripttools printsupport
equals(QT_MAJOR_VERSION, 5): QT += widgets webkitwidgets

minQtVersion(5, 4, 0){
    QT -= webkit webkitwidgets
    QT += webengine webenginewidgets
}
