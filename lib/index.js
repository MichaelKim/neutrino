const ons = {};

function emit(eventName, ...args) {
  if (typeof ons[eventName] === 'function') {
    ons[eventName](...args);
  }
}

function on(eventName, callback) {
  ons[eventName] = callback;
}

function n_ready() {
  emit('ready');
}

function log(...data) {
  n_log(...data);
}

function quit() {
  n_quit();
}

class BrowserWindow {
  constructor(opts) {
    this.id = Date.now();
    n_createWindow(opts, this.id);
  }

  loadURL(url) {
    n_navigate(url, this.id);
  }

  on(eventName, callback) {
    // TODO
  }
}

module.exports = {
  app: {
    on,
    log,
    quit,
    n_ready
  },
  BrowserWindow
};
