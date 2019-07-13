/*
Two places where messages can be recieved from:
- JS
  - A request is made to query a C++ API (e.g. fs)
- C++
  - Emitting an event, or a response to a previously made request
*/

onmessage = e => {
  if (e.data.type === 'emit') {
    emit(e.data.eventName, e.data.args);
  }
};

const ons = {};
const emits = {};

function on(eventName, callback) {
  if (emits[eventName] != null) {
    emits[eventName].forEach(e => callback(...e));
  }
  ons[eventName] = callback;
}

function log(...data) {
  postMessage(
    JSON.stringify({
      type: 'log',
      data
    })
  );
}

function emit(eventName, args = []) {
  if (typeof ons[eventName] === 'function') {
    ons[eventName](...args);
  }
}

class BrowserWindow {
  constructor(opts) {}
}

// Remote
class Menu {
  constructor() {
    this.items = [];
  }

  append(item) {
    this.items.push(item);
  }

  popup() {
    // postMessage(
    //   JSON.stringify({
    //     type: 'context-menu'
    //   })
    // );
  }
}

module.exports = {
  app: {
    on,
    log
  },
  BrowserWindow,
  remote: {
    Menu
  }
};
