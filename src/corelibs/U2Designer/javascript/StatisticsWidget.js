/*WorkerInfo {
    QString actorId;
    QString actor;
    int ticks;
    qint64 timeMks;
    int countOfProducedData;
  };
*/
function StatisticsWidget(containerId){
  //create parent widget
  addWidget("<span lang=\"en\" class=\"translatable\">Common Statistics</span>" + "<span lang=\"ru\" class=\"translatable\">Общая статистика</span>", "overview_tab", 1, containerId);
  TableWidget.apply(this, arguments); //inheritance 
  //private
  var self = this;
  //protected
  this._useEmptyRows = false;
  //public
  this.widths = [40, 30, 30];
  this.headers = ["<span lang=\"en\" class=\"translatable\">Element</span>" + "<span lang=\"ru\" class=\"translatable\">Элемент</span>",
                  "<span lang=\"en\" class=\"translatable\">Elapsed time</span>" + "<span lang=\"ru\" class=\"translatable\">Прошедшее время</span>",
                  "<span lang=\"en\" class=\"translatable\">Output messages</span>" + "<span lang=\"ru\" class=\"translatable\">Выходные сообщения</span>"
                  ];
  
  this.sl_workerStatsInfoChanged = function(info){
    if (!self._isContainerExists()) {
      agent.sl_onJsError("Can't find container by id = " + self._containerId + "!");
      return;
    }
    self._updateRow(id(info.actorId), createRowByWorker(info));
  };
  this.sl_workerStatsUpdate = function(workersStatisticsInfo){
    if (!self._isContainerExists()) {
      agent.sl_onJsError("Can't find container by id = " + self._containerId + "!");
      return;
    }
    workersStatisticsInfo.forEach(function(workerInfo){
      self._updateRow(id(workerInfo.actorId), createRowByWorker(workerInfo));
    });
  };
  //private
  function createRowByWorker(info){
    var result = [];

    result.push(wrapLongText(/*m->actorName(*/info.actor));
    result.push(timeStr(info.timeMks));// << timeStr(info.timeMks);
    result.push(info.countOfProducedData);
    return result;
  }
  function timeStr(timeMks){
    var date = new Date(timeMks/1000);
    var milliseconds = date.getUTCMilliseconds() > 99 ? date.getUTCMilliseconds() : (date.getUTCMilliseconds() > 9 ? "0"+date.getUTCMilliseconds() : "00"+date.getUTCMilliseconds());
    var seconds = date.getUTCSeconds() > 9 ? date.getUTCSeconds() : "0"+date.getUTCSeconds();
    var minutes = date.getUTCMinutes() > 9 ? date.getUTCMinutes() : "0"+date.getUTCMinutes();
    var hours = date.getUTCHours() > 9 ? date.getUTCHours() : "0"+date.getUTCHours();
    return hours + ":" + minutes + ":" + seconds + "." + milliseconds;
  }
  function id(actor){
      return "stw_" + actor;
  }
  //constructor code
  this._createTable();
  this._fillTable();
  showOnlyLang(agent.lang); //translate labels
}
