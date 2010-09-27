var Tests = {
  "It should correctly fetch list of elements": function(t) {
    var elements = Komissar("h1, p");
    t.assert(["h1", "p"].sort(), elements.map(function(e) { return e.tagName }).sort());
  },

  "It should correctly fetch list of elements": function(t) {
    var elements = Komissar(".div1, .div2, .div3");
    t.assert(["div", "div", "div"].sort(), elements.map(function(e) { return e.tagName }).sort());
  }
};


var TestRunner = {
  assert: function(expr, messageArray) {
    if (!expr) {
      if (messageArray && messageArray.length > 0) {
        console.error.apply(console, messageArray);
      }

      throw "Assertion failed";
    }
  },

  assertEqual: function(expected, actual) {
    this.assert(expected === actual, ["Expected", actual, "will be equal", expected]);
  },

  runTests: function(tests) {
    var testsCount = 0;
    var successfullCount = 0;

    console.info("Running test suite");
    for(var testName in tests) {
      var currentResult = false;
      try {
        tests[testName](this);
        currentResult = true;
      } catch (x) {
        // nothing to do here
      }

      testsCount += 1;
      successfullCount += (currentResult === true ? 1 : 0);
      console[currentResult === true ? "log" : "error"](testName + ": " + (currentResult === true ? "OK" : "FAILED"));
    }

    console.info("Total tests:", testsCount, "Errors: ", testsCount - successfullCount);
  }
};

window.onload = function() {
  TestRunner.runTests(Tests);
};