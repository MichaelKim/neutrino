const ons = {};
const emits = {};

function on(eventName, callback) {
  if (emits[eventName] != null) {
    emits[eventName].forEach(e => callback(...e));
  }
  ons[eventName] = callback;

  postMessage(
    JSON.stringify({
      type: 'on',
      eventName
    })
  );
}

function log(...data) {
  postMessage(
    JSON.stringify({
      type: 'log',
      data
    })
  );
}

onmessage = e => {
  if (e.data.type === 'emit') {
    emit(e.data.eventName, e.data.args);
  }
};

function emit(eventName, args = []) {
  if (typeof ons[eventName] === 'function') {
    ons[eventName](...args);
  }
}

class BrowserWindow {
  constructor(opts) {}
}

module.exports = {
  app: {
    on,
    log
  },
  BrowserWindow
};
