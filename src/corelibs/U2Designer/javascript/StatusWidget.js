/*enum U2LANG_EXPORT TaskState {
    RUNNING,
    RUNNING_WITH_PROBLEMS,
    FINISHED_WITH_PROBLEMS,
    CANCELLED,
    FAILED,
    SUCCESS
};
*/
function StatusWidget(containerId){
  //create parent widget
  addWidget("<span lang=\"en\" class=\"translatable\">Workflow task</span>" + "<span lang=\"ru\" class=\"translatable\">Задача схемы</span>", "overview_tab", 1, containerId);
  DashboardWidget.apply(this, arguments); //inheritance 
  //private
  var self = this;
  //public
  this.sl_progressChanged = function(progress){ //int progress
    if (!self._isContainerExists()) {
      agent.sl_onJsError("Can't find container by id = " + self._containerId + "!");
      return;
    }
    bar = self._container.getElementsByClassName("bar");
  	if (bar === null) {
          agent.sl_onJsError("Can't find element by class = bar!");
          return;
      }
  	bar[0].style.width = progress+"%";
  };

  this.sl_taskStateChanged = function(state){ //Monitor::TaskState
    if (!self._isContainerExists()) {
      agent.sl_onJsError("Can't find container by id = " + self._containerId + "!");
      return;
    }
    updatePrivateVariables();

    var isFinished = false;
    if ("RUNNING" == state) {
      running();
    } else if ("RUNNING_WITH_PROBLEMS" == state) {
      runningWithProblems();
    } else if ("FINISHED_WITH_PROBLEMS" == state) {
      isFinished = true;
      finishedWithProblems();
    } else if ("FAILED" == state) {
      isFinished = true;
      failed();
    } else if ("SUCCESS" == state) {
      isFinished = true;
      success();
    } else {
      isFinished = true;
      canceled();
    }
    
    var hint = document.getElementById("load-btn-hint-container");//only for debug
    if(isFinished && hint === null){
      showLoadButton(agent.showHint);
    }
    showOnlyLang(agent.lang); //translate labels
  };
  //private
  function running(){
    statusBar.classList.add("alert-info");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task is in progress...</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы в процессе...</span>";
  }
  function runningWithProblems(){
    statusBar.classList.remove("alert-info");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task is in progress. There are problems...</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы в процессе. Есть проблемы...</span>";
  }
  function finishedWithProblems(){
    statusBar.classList.remove("alert-info");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task has been finished with warnings!</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы завершилась с предупреждениями!</span>";
  }
  function failed(){
    statusBar.classList.remove("alert-info");
    statusBar.classList.add("alert-error");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task has been finished with errors!</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы завершилась с ошибками!</span>";
  }
  function success(){
    statusBar.classList.remove("alert-info");
    statusBar.classList.add("alert-success");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task has been finished successfully!</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы завершилась без ошибок!</span>";
  }
  function canceled(){
    statusBar.classList.remove("alert-info");
    statusMessage.innerHTML = "<span lang=\"en\" class=\"translatable\">The workflow task has been canceled!</span>" +
                              "<span lang=\"ru\" class=\"translatable\">Задача выполнения схемы была отменена!</span>";
  }
  function updatePrivateVariables(){
      statusBar = document.getElementById("status-bar");
      statusMessage = document.getElementById("status-message");
  }
  //constructor
  var content = "<div class=\"well well-small vlayout-item\">" +
                "<span lang=\"en\" class=\"translatable\">Time</span>" +
                "<span lang=\"ru\" class=\"translatable\">Время</span>" +
                " <span id=\"timer\"></span>" +
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
  self._container.innerHTML += content;
  var statusBar = document.getElementById("status-bar");
  var statusMessage = document.getElementById("status-message");
  self.sl_progressChanged(0);
  running();
  showOnlyLang(agent.lang); //translate labels
}
