/*Problem {
    QString message;
    QString actorId;
    QString actorName;
    QString type;
    QString port;

    bool operator== (const Problem &other) const;

    static const QString U2_ERROR;//   = "error";
    static const QString U2_WARNING;// = "warning";
    static const QString U2_INFO;//    = "info";
  }
*/
function ProblemsWidget(containerId){ // may be add first problem as second argument function ProblemsWidget(containerId, firstProblemInfo)
  addWidget("<span lang=\"en\" class=\"translatable\">Notifications</span>" + "<span lang=\"ru\" class=\"translatable\">Уведомления</span>", "overview_tab", 0, containerId);
  TableWidget.apply(this, arguments); //inheritance 
  //private
  var self = this;
  //public
  this.widths = [10, 30, 60];
  this.headers = ["<span lang=\"en\" class=\"translatable\">Type</span>" + "<span lang=\"ru\" class=\"translatable\">Тип</span>",
                  "<span lang=\"en\" class=\"translatable\">Element</span>" + "<span lang=\"ru\" class=\"translatable\">Элемент</span>",
                  "<span lang=\"en\" class=\"translatable\">Message</span>" + "<span lang=\"ru\" class=\"translatable\">Сообщение</span>"
                  ];
  this.sl_newProblem = function (problemInfo, count){
    if (!self._isContainerExists()) {
      agent.sl_onJsError("Can't find container by id = " + self._containerId + "!");
      return;
    }
    if (id(problemInfo) in self._rows) {
        self._updateRow(id(problemInfo), createRow(problemInfo, /*multi row*/ true, count));
    } else {
        self._addRow(id(problemInfo), createRow(problemInfo));
    }
  };

  this.problemImage = function (problemInfo){
    var image = "qrc:///U2Lang/images/";
    if ("error" === problemInfo.type) {
        image += "error.png";
        tooltip = (agent.lang !== "ru") ? "Error" : "Ошибка";
    } else if ("warning" === problemInfo.type) {
        image += "warning.png";
        tooltip = (agent.lang !== "ru") ? "Warning" : "Предупреждение";
    } else if ("info" === problemInfo.type) {
        image = "qrc:///core/images/info.png";
        tooltip = (agent.lang !== "ru") ? "Information" : "Информация";
    } else {
        agent.sl_onJsError("Unknown type: " + problemInfo.type, "");
    }
    return "<img src=\"" + image + "\" title=\"" + tooltip + "\" class=\"problem-icon\"/>";
  };
  //protected
  this._createRow = function(rowData) {
    var row = "";
    rowData.forEach(function(item) {
      row += "<td style=\"word-wrap: break-word\">" + item + "</td>";
    });
    return row;
  };
  //private
  function createRow(info, multi, count){
    multi = multi || false;
    count = count || 1;
    var result = [];
    var prefix = "";
    if (multi) {
        prefix = "(" + count + ") ";
    }

    result.push(self.problemImage(info));
    result.push(self.wrapLongText(info.actorName));
    result.push(getTextWithWordBreaks(prefix + info.message));

    return result;
  }
  function getTextWithWordBreaks(text){
    var textWithBreaks = text;
    textWithBreaks = textWithBreaks.replace("\\", "\\<wbr>").replace("/", "/<wbr>");
    return textWithBreaks;
  }
  function id(info){
    return info.actorId + info.message;
  }
  //constructor code
  this._createTable();
  showOnlyLang(agent.lang); //translate labels
}
