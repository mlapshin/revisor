var commands = {
  session_start: {
    name: "session.start"
  },

  session_tab_create: {
    name: "session.tab.create",
    session_index: 0
  },

  session_tab_visit: {
    name: "session.tab.visit",
    session_index: 0,
    tab_index: 0,
    url: "http://google.com/"
  }
};

var commandsToSend = [];

$(document).ready(function() {
  var commandsHtml = "";
  $.each(commands, function(k, v) {
    commandsHtml += "<option value='" + k + "'>" + k.replace(/\_/g, ".") + "</option>";
  });

  $("#command_select").html(commandsHtml);
  $("#command_select").change(function() {
    $("#commands").html(JSON.stringify(commands[this.value], null, '  '));
  });

  $("#submit_button").bind("click", function() {
    commandsToSend = $("#commands").val().split(/\n\n/);
    console.log("Command queue", commandsToSend);
    var cmd = commandsToSend.shift();

    if (cmd) {
      $("#command").val(cmd);
    }

    return true;
  });
});

function postNextCommand() {
  var cmd = commandsToSend.shift();
  if (cmd) {
    console.log("Post next command");
    console.log($("#command_form").first());
    $("#command").val(cmd);
    $("#command_form").first().submit();
  }
}