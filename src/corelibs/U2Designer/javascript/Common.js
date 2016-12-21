/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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
 
function wrapLongText(text) {
    return '<div class="long-text" title="' + text + '">' + text + '</div>';
}

/**
 * Hides the hint of the load scheme button and
 * specifies not to show it anymore.
 */
function hideLoadBtnHint() {
    var hint = document.getElementById("load-btn-hint-container");
    if (null !== hint) {
        hint.parentNode.removeChild(hint);
    }
}

/**
 * Shows a button to load the original scheme.
 * If "showHint" is "true", shows a hint about the button usage.
 */
function showLoadButton(showHint) {
    var menuLine = document.getElementsByClassName("dash-menu-line")[0];
    var btnDef = "<button class='btn load-btn' onclick='agent.loadSchema()' title='Load dashboard workflow'><div /></button>";
    menuLine.insertAdjacentHTML('beforeend', btnDef);

    if (showHint === true) {
        var hintDef =
        "<div id='load-btn-hint-container'>" +
            "<div id='load-btn-hint' class='popover fade bottom in' style='display: block'>" +
                "<div class='arrow' style='left: 91%'></div>" +
                "<div class='popover-content'>" +
                    "<span lang=\"en\" class=\"translatable\">You can always open the original workflow for your results by clicking on this button.</span>" +
                    "<span lang=\"ru\" class=\"translatable\">Вы всегда можете открыть исходную вычислительную схему для ваших результатов, нажав на эту кнопку.</span>" +
                    "<div style='text-align: center;'>" +
                            "<button class='btn' onclick='agent.hideLoadButtonHint()' style='margin-bottom: 4px; margin-top: 6px;'><span lang=\"en\" class=\"translatable\">OK, got it!</span><span lang=\"ru\" class=\"translatable\">Хорошо!</span></button>" +
                    "</div>" +
                "</div>" +
            "</div>" +
        "</div>";
        menuLine.insertAdjacentHTML('beforeend', hintDef);
    }
}

function showFileButton(url, disabled) {
    if (disabled === true) {
        disabled = 'disabled';
    } else {
        disabled = '';
    }

    if (url.length === 0)
        return "";
    var fileName = url.slice(url.lastIndexOf('/') + 1, url.length);
    var path = url.slice(0, url.lastIndexOf('/') + 1);
    var button = 
        '<div class="file-button-ctn">' +
            '<div class="btn-group full-width file-btn-group">' + 
                '<button class="btn full-width long-text" onclick="agent.openUrl(\'' + url + '\')"' +
                                disabled + '>' + fileName +
                '</button>' +
                    '<button class="btn dropdown-toggle" data-toggle="dropdown">' + 
                        '<span class="caret"></span>' +
                    '</button>' +
                '<ul class="dropdown-menu full-width">' +
                        '<li><a style="white-space: normal;" onclick="agent.openByOS(\'' + path + '\')"><span lang=\"en\" class=\"translatable\">Open containing folder</span><span lang=\"ru\" class=\"translatable\">Открыть директорию, содержащую файл</span></a></li>' +
                        '<li><a style="white-space: normal;" onclick="agent.openByOS(\'' + path + fileName + '\')"><span lang=\"en\" class=\"translatable\">Open by operating system</span><span lang=\"ru\" class=\"translatable\">Открыть при помощи операционной системы</span></a></li>' + 
                    '</ul>' +
            '</div>' +
        '</div>';
    return button;
}

function showFileMenu(url) {
    if (url.length === 0)
        return "";
    var fileName = url.slice(url.lastIndexOf('/') + 1, url.length);
    var path = url.slice(0, url.lastIndexOf('/') + 1);
    var li = 
    '<li class="file-sub-menu dropdown-submenu left-align">' +
      '<a tabindex="-1" href="#" onclick="agent.openUrl(\'' + url + '\')" title="' + url + '">' + fileName + '</a>' +
      '<ul class="dropdown-menu ">' +
        '<li><a href="#" onclick="agent.openByOS(\'' + path + '\')"><span lang=\"en\" class=\"translatable\">Open containing folder</span><span lang=\"ru\" class=\"translatable\">Открыть директорию, содержащую файл</span></a></li>' +
        '<li><a href="#" onclick="agent.openByOS(\'' + path + fileName + '\')"><span lang=\"en\" class=\"translatable\">Open by operating system</span><span lang=\"ru\" class=\"translatable\">Открыть при помощи операционной системы</span></a></li>' +
      '</ul></li>';
    return li;
}

function addTab(tabId, tabName) {
    var tabsList = document.getElementsByClassName("nav nav-pills dash-nav")[0];
    var newTab = "<li class=''><a href='" + tabId + "' data-toggle='tab' class='dash-tab-name'>" + tabName + "</a></li>";
    tabsList.insertAdjacentHTML('beforeend', newTab);
}

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