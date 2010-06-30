var commands = {
  session_start: {
    name: "session.start"
  },

  session_tab_create: {
    name: "session.tab.create",
    session_index: 0
  }
};

$(document).ready(function() {
  $("#command_select").change(function() {
    console.log(this);
    $("#command").html(JSON.stringify(commands[this.value], null, '\t'));
  });
});