//Dashboard.cpp
function addWidget(title, dashTab, cntNum, id) {
    var tabContainer = document.getElementById(dashTab);
    if (tabContainer === null) {
        agent.sl_onJsError("Can't find the tab container!");
        return;
    }
    var hasInnerContainers = true;
    var InputDashTab = "input_tab";
    var ExternalToolsTab = "ext_tools_tab";
    if (InputDashTab == dashTab || ExternalToolsTab == dashTab) {
        hasInnerContainers = false;
    }
    var mainContainer = tabContainer;
    if (hasInnerContainers) {
        var left = true;
        if (0 === cntNum) {
            left = true;
        } else if (1 == cntNum) {
            left = false;
        } else if (containerSize(tabContainer, ".left-container") <= containerSize(tabContainer, ".right-container")) {
            left = true;
        } else {
            left = false;
        }

        var elements = tabContainer.getElementsByClassName(left ? "left-container" : "right-container");
        if (elements[0] === null) {
            agent.sl_onJsError("Can't find a container inside a tab!");
            return;
        }
        mainContainer = elements[0];
        mainContainer.innerHTML = mainContainer.innerHTML +"<div class=\"widget\">" +
            "<div class=\"title\"><div class=\"title-content\">" + title + "</div></div>" +
            "<div class=\"widget-content\" id=\"" + id + "\"></div>" +
            "</div>";
    }
}

//ResourcesWidget.cpp
function addContent(containerId, content) {
	var container = document.getElementById(containerId);
    if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
	container.innerHTML = container.innerHTML + content;
}

function changeProgress(value) {
	var container = document.getElementById("overview_tab");
    if (container === null) {
        agent.sl_onJsError("Can't find container by id = overview_tab!");
        return;
    }
	bar = container.getElementsByClassName("bar");
	if (bar === null) {
        agent.sl_onJsError("Can't find element by class = bar!");
        return;
    }
	bar[0].style.width = value+"%";
}

function addClassById(id, classToAdd) {
	var d = document.getElementById(id);
	if (d === null) {
        agent.sl_onJsError("Can't find element by id = " + id + "!");
        return;
    }
	d.className += classToAdd;
}

function removeClassById(id, classToRemove) {
	var d = document.getElementById(id);
	if (d === null) {
        agent.sl_onJsError("Can't find element by id = " + id + "!");
        return;
    }
	d.removeClass(classToRemove);
}

function addResWidgetContent(time) {
    var content = "<div class=\"well well-small vlayout-item\">" +
        time + " <span id=\"timer\"></span>" +
        "</div>" +
        "<div class=\"progress-wrapper vlayout-item\">" +
        "<div class=\"progress-container\">" +
        "<div id=\"progressBar\" class=\"progress small-bar\">" +
        "<div class=\"bar\" style=\"width: 0%;\"></div>" +
        "</div>" +
        "</div>" +
        "</div>" +
        "<div id=\"status-bar\" class=\"vlayout-item alert\">" +
        "<p id=\"status-message\"/>" +
        "</div>";
    addContent("resourceWidget", content);
}

//TableWidget.cpp
function clearTable(containerId) {
    container = document.getElementById(containerId);
    if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
    container.parentNode.removeChild(container);
}

function createTable(containerId, widthsArray, headersArray) {
	var content = "<table class=\"table table-bordered table-fixed\">";
	widthsArray.forEach(function(item) {
	  content += "<col width=\"" + item + "%\" />";
	});
	content += "<thead><tr>";
	headersArray.forEach(function(item) {
	  content += "<th><span class=\"text\">" + item + "</span></th>";
	});
	content += "<thead><tr>";
	content += "<tbody scroll=\"yes\"/>";
    content += "</table>";
	addContent(containerId, content);
}

function addEmptyRows(containerId, minRows, headersCounter) {
	container = document.getElementById(containerId);
	if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
	body = container.getElementsByTagName('tbody')[0];
	rowIdx = 0;
	while (rowIdx < minRows) {
		var row = "<tr class=\"empty-row\">";
		for(i = 0; i < headersCounter; i++) {
		  row += "<td>&nbsp;</td>";
		}
		row += "</tr>";
		body.innerHTML += row;
		rowIdx++;
	}
}

function createRow(rowArray) {
    var row = "";
    rowArray.forEach(function(item) {
	  row += "<td>" + item + "</td>"
	});
    return row;
}

function addRow(containerId, rowArray, rowId) {
	var container = document.getElementById(containerId);
	if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
    var row = "<tr class=\"filled-row\" id=\"";
    row += rowId + "\">";
    row += createRow(rowArray);
    row += "</tr>";
    
	var body = container.getElementsByTagName('tbody')[0];
	var emptyRows = body.getElementsByClassName("empty-row");
	if (emptyRows.length === 0) {
		body.outerHTML += row;
	} else {
		emptyRows[0].outerHTML = row;
	}	
}

function updateRow(containerId, rowArray, rowId) {
    var row = document.getElementById(rowId);
    if (row === null) {
        addRow(containerId, rowArray, rowId);
    }else{
        row.innerHTML = createRow(rowArray);
    }
}

function wrapLongText(text) {
    return "<div class=\"long-text\" title=\"" + text + "\">" + text + "</div>";
}

//OutputFiles.cpp
function createRowByFile(relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation, actorName) {
    var rowArray = [];
    rowArray[0] = createFileButton(relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation);
    rowArray[1] = wrapLongText(actorName);
    return rowArray;
}

function createFileButton(relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation) {
    var content = "<div class=\"file-button-ctn\">" +
        "<div class=\"btn-group full-width file-btn-group\">" +
        "<button class=\"btn full-width long-text\" onclick=" + 
        onClickAction(relativeURLPath, openBySystem) +
        "onmouseover=\"this.title=agent.absolute('"+ 
        relativeURLPath  +
        "')\">" +
        fileName + 
        "</button><button class=\"btn dropdown-toggle\" data-toggle=\"dropdown\">" +
        "<span class=\"caret\"></span></button>" +
        createActionsSubMenu(relativeURLPath, relativeDirPath, true, openBySystem, openByOsTranslation, openContainingDirTranslation) + 
        "</div></div>";
    return content;
}

function onClickAction(relativeURLPath, openBySystem) {
    var content = (openBySystem === true ) ? "\"agent.openByOS('" + relativeURLPath + "')\"":"\"agent.openUrl('" + relativeURLPath +  "')\"";
    return content;
}

function createActionsSubMenu(relativeURLPath, relativeDirPath, fullWidth, openBySystem, openByOsTranslation, openContainingDirTranslation) {
    var openFileByOsAction = (openBySystem === true) ? "" :"<li><a href=\"#\" onclick=\"agent.openByOS('" + relativeURLPath +
    "')\">" + openByOsTranslation + "</a></li>";
    var fw = fullWidth ? "full-width" : "";
    var content = "<ul class=\"dropdown-menu " + fw +
        "\"><li><a href=\"#\" onclick=\"agent.openByOS('" + relativeDirPath
        + "')\">" + openContainingDirTranslation
        +" </a></li>" + openFileByOsAction +
        "</ul>";
    return content;
}

function createFileListButton(containerId, filesTranslation, actorName, rowId) {
    var filesButton = "<div class=\"btn-group full-width\">" +
            "<button class=\"files-btn btn dropdown-toggle full-width\" data-toggle=\"dropdown\" href=\"#\">" +
                buttonLabel(0, filesTranslation) +
            "</button>" +
            "<ul class=\"files-menu dropdown-menu full-width\"/>" +
        "</div>";
    var rowArray = [];
    rowArray[0] = filesButton;
    rowArray[1] = wrapLongText(actorName);
    updateRow(containerId, rowArray, rowId);
}

function buttonLabel(filesCount, filesTranslation) {
    var content = filesCount + " " + filesTranslation + " <span class=\"caret\"></span>";
    return content;
}

function createFilesButton(containerId, filesTranslation, actorName, rowId, filepathArr) {
    //cpp parameters actorId, files
    createFileListButton(containerId, filesTranslation, actorName, rowId);
    filepathArr.forEach(function(item) {
	    addFileMenu(containerId, rowId, relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation,  filesTranslation, actorName, filepathByActorNameArr);
	});
}

function addFileMenu(containerId, rowId, relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation,  filesTranslation, actorName, filepathByActorNameArr) {
	var row = document.getElementById(rowId);
	if (row === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
    var buttons = row.getElementsByClassName("file-button-ctn");
    if (buttons.length > 0) {
        createFilesButton(containerId, filesTranslation, actorName, rowId, filepathArr);
    } else {
        var button = row.getElementsByClassName("files-btn");
        var menu = row.getElementsByClassName("files-menu");
        var count = row.getElementsByClassName("file-sub-menu").length + 1;
        button.innerHTML = buttonLabel(count , filesTranslation);
        menu.innerHTML += createFileSubMenu(relativeURLPath, relativeDirPath, false, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation);
    }
}

function createFileSubMenu(relativeURLPath, relativeDirPath, openBySystem, fileName, openByOsTranslation, openContainingDirTranslation) {
    var content = "<li class=\"file-sub-menu dropdown-submenu left-align\">" +
        "<a tabindex=\"-1\" href=\"#\" onclick=" + 
        onClickAction(relativeURLPath, openBySystem) +
        " title=\"" +
        relativeURLPath +
        "\">" +
        fileName +
        "</a>" +
        createActionsSubMenu(relativeURLPath, relativeDirPath, false, openBySystem, openByOsTranslation, openContainingDirTranslation) +
        "</li>";
    return content;
}

//ParametersWidget.cpp
function pwInitAndActiveTab_webengine(container, activeTabName, activeTabId) {
    var container = document.getElementById(containerId);
    if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
    var mainHtml =
        '<div class="tabbable tabs-left">' +
            '<ul class="nav nav-tabs params-nav-tabs">' +
                '<li class="active"><a href="#' + activeTabId + '" data-toggle="tab">' + activeTabName + '</a></li>' +
            '</ul>' +
            '<div class="tab-content params-tab-content">' +
                '<div class="tab-pane active" id="' + activeTabId + '">' +
                    '<table class="table table-bordered table-fixed param-value-column">' +
                        '<col width="45%">' +
                        '<col width="55%">' +
                        '<thead>' +
                            '<tr>' +
                                '<th><span class="text">Parameter</span></th>' +
                                '<th><span class="text">Value</span></th>' +
                            '</tr>' +
                        '</thead>' +
                        '<tbody scroll="yes">' +
                        '</tbody>' +
                    '</table>' +
                '</div>' +
            '</div>' +
        '</div>';
    
    container.innerHTML = mainHtml;
}

/**
 * Searches for the ParametersWidget initial layout in the container and
 * appends a new common (non-active) tab without parameters.
 */
function pwAddTab_webengine(container, tabName, tabId) {
    var container = document.getElementById(containerId);
    if (container === null) {
        agent.sl_onJsError("Can't find container by id = " + containerId + "!");
        return;
    }
    var tabsList = container.getElementsByClassName("params-nav-tabs")[0];
    var tabsContent = container.getElementsByClassName("params-tab-content")[0];

    var newTabsListEntry = '<li><a href="#' + tabId + '" data-toggle="tab">' + tabName + '</a></li>';
    var newTabsContentEntry =
        '<div class="tab-pane" id="' + tabId + '">' +
            '<table class="table table-bordered table-fixed param-value-column">' +
                '<col width="45%">' +
                '<col width="55%">' +
                '<thead>' +
                    '<tr>' +
                        '<th><span class="text">Parameter</span></th>' +
                        '<th><span class="text">Value</span></th>' +
                    '</tr>' +
                '</thead>' +
                '<tbody scroll="yes">' +
                '</tbody>' +
            '</table>' +
        '</div>';
    
    tabsList.insertAdjacentHTML('beforeend', newTabsListEntry);
    tabsContent.insertAdjacentHTML('beforeend', newTabsContentEntry);
}
