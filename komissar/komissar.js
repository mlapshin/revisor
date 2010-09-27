function NodeListProxy(selectorOrNodeListProxy) {
  this.nodeList = document.querySelectorAll(selectorOrNodeListProxy);
}

NodeListProxy.prototype = {
  nodeList: null,

  each: function(f) {
    for(var i = 0; i < this.nodeList.length; i++) {
      f(this.nodeList[i]);
    }
  },

  map: function(f) {
    var result = [];
    for(var i = 0; i < this.nodeList.length; i++) {
      result.push(f(this.nodeList[i]));
    }

    return result;
  },

  length: function() {
    return this.nodeList.length;
  },

  pos: null,
  size: null,

  attr: function(attrName) {
    if (this.length() > 0) {
      return this.nodeList[attr];
    } else {
      return undefined;
    }
  },

  value: function(newValue) {
    if (arguments.length == 0) {
      this.attr('value');
    } else {
      if (this.length() > 0 && this.nodeList[0][value]) {
      }
    }
  },

  html: null,
  css: null
};

function Komissar(selector) {
  return new NodeListProxy(selector);
}
