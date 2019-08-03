"use strict";

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var ons = {};

function emit(eventName) {
  if (typeof ons[eventName] === 'function') {
    for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
      args[_key - 1] = arguments[_key];
    }

    ons[eventName].apply(ons, args);
  }
}

function on(eventName, callback) {
  ons[eventName] = callback;
}

function n_ready() {
  emit('ready');
}

function log() {
  n_log.apply(void 0, arguments);
}

function quit() {
  n_quit();
}

var BrowserWindow =
/*#__PURE__*/
function () {
  function BrowserWindow(opts) {
    _classCallCheck(this, BrowserWindow);

    this.id = Date.now(); // Parse hex value

    if (opts.backgroundColor != null) {
      var rgb = opts.backgroundColor;
      var r = 255,
          g = 255,
          b = 255,
          a = 255;

      if (rgb.length === 4) {
        // #rgb
        var _rgb$match$map = rgb.match(/\w/g).map(function (x) {
          return parseInt(x + x, 16);
        });

        var _rgb$match$map2 = _slicedToArray(_rgb$match$map, 3);

        r = _rgb$match$map2[0];
        g = _rgb$match$map2[1];
        b = _rgb$match$map2[2];
      } else if (rgb.length === 7) {
        // #rrggbb
        var _rgb$match$map3 = rgb.match(/\w\w/g).map(function (x) {
          return parseInt(x, 16);
        });

        var _rgb$match$map4 = _slicedToArray(_rgb$match$map3, 3);

        r = _rgb$match$map4[0];
        g = _rgb$match$map4[1];
        b = _rgb$match$map4[2];
      } else if (rgb.length === 9) {
        // #aarrggbb
        var _rgb$match$map5 = rgb.match(/\w\w/g).map(function (x) {
          return parseInt(x, 16);
        });

        var _rgb$match$map6 = _slicedToArray(_rgb$match$map5, 4);

        a = _rgb$match$map6[0];
        r = _rgb$match$map6[1];
        g = _rgb$match$map6[2];
        b = _rgb$match$map6[3];
      }

      opts.backgroundColor = null;
      opts._bgColorR = r;
      opts._bgColorG = g;
      opts._bgColorB = b;
      opts._bgColorA = a;
    }

    n_createWindow(opts, this.id);
  }

  _createClass(BrowserWindow, [{
    key: "loadURL",
    value: function loadURL(url) {
      n_navigate(url, this.id);
    }
  }, {
    key: "on",
    value: function on(eventName, callback) {// TODO
    }
  }]);

  return BrowserWindow;
}();

module.exports = {
  app: {
    on: on,
    log: log,
    quit: quit,
    n_ready: n_ready
  },
  BrowserWindow: BrowserWindow
};