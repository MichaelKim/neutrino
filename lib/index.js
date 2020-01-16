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

    // Parse hex value
    if (opts.backgroundColor != null) {
      const rgb = opts.backgroundColor;

      let r = 255,
        g = 255,
        b = 255,
        a = 255;

      if (rgb.length === 4) {
        // #rgb
        [r, g, b] = rgb.match(/\w/g).map(x => parseInt(x + x, 16));
      } else if (rgb.length === 7) {
        // #rrggbb
        [r, g, b] = rgb.match(/\w\w/g).map(x => parseInt(x, 16));
      } else if (rgb.length === 9) {
        // #aarrggbb
        [a, r, g, b] = rgb.match(/\w\w/g).map(x => parseInt(x, 16));
      }

      opts.backgroundColor = null;

      opts._bgColorR = r;
      opts._bgColorG = g;
      opts._bgColorB = b;
      opts._bgColorA = a;
    }

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
