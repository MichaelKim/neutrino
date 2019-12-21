#include "json.hpp"
#include "quickjs-libc.h"
#include "webview.hpp"

#include <experimental/filesystem>  // fs::absolute, fs::exists
#include <fstream>                  // File read / write
#include <iostream>                 // Debug output
#include <sstream>                  // File input to string
#include <unordered_map>            // std::vector

#ifdef WEBVIEW_WIN
#include <shellapi.h>  // For CommandLineToArgvW
#endif

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

void usage() {
  std::cout << "Neutrino" << std::endl;
  std::cout << "Usage: neutrino [path]" << std::endl << std::endl;
}

/*
Argument: directory path
- main: path/main.js
- __dirname: path

Default path: ./app/
- main: ./app/main.js
- __dirname: ./app
*/

// Utility Functions
#ifdef WEBVIEW_WIN
#define Cout std::wcout
std::wstring format(const std::string &str) {
  if (str.empty()) {
    return std::wstring();
  }

  int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstr(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
  return wstr;
}
std::string normalize(const std::wstring &wstr) {
  if (wstr.empty()) {
    return std::string();
  }

  int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL,
                                 0, NULL, NULL);
  std::string str(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size,
                      NULL, NULL);
  return str;
}
std::wstring getPath() {
  if (fs::exists(fs::absolute("app") / "main.js")) {
    wv::String path = fs::absolute("app").wstring();
    return path;
  }

  int argc;
  wchar_t **arglist = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc != 2) {
    return std::wstring();
  }

  // Local file path
  wv::String path(arglist[1]);
  return fs::absolute(path).wstring();
}
#else
#define Cout std::cout
inline std::string format(std::string str) { return str; }
inline std::string normalize(std::string str) { return str; }
std::string getPath(int argc, char **argv) {
  if (fs::exists(fs::absolute("app") / "main.js")) {
    return fs::absolute("app").string();
  }

  if (argc != 2) {
    return std::string();
  }

  // Local file path
  wv::String path(argv[1]);
  return fs::absolute(path).string();
}
#endif

int getIntPropDefault(JSContext *ctx, JSValueConst obj, const char *prop,
                      int def) {
  JSValue val = JS_GetPropertyStr(ctx, obj, prop);
  if (JS_IsUndefined(val)) {
    return def;
  }
  int ret;
  if (JS_ToInt32(ctx, &ret, val)) {
    return def;
  }
  return ret;
}

wv::String getStrPropDefault(JSContext *ctx, JSValueConst obj, const char *prop,
                             std::string def) {
  JSValue val = JS_GetPropertyStr(ctx, obj, prop);
  if (JS_IsUndefined(val)) {
    return format(def);
  }
  return format(std::string(JS_ToCString(ctx, val)));
}

// Neutrino API

std::tuple<wv::String, wv::String> readFile(std::string filepath) {
  try {
    std::cout << "current dir: " << fs::current_path() << std::endl;
    std::cout << "reading file: " << filepath << std::endl;

    std::ifstream input(filepath.c_str());
    if (!input.is_open()) {
      return {Str(""), Str("Missing File")};
    }

    std::stringstream sstr;
    sstr << input.rdbuf();

    wv::String contents = format(sstr.str());

    return {contents, Str("")};
  } catch (json::exception &ex) {
    wv::String err = format(ex.what());
    return {Str(""), err};
  }
}

void writeFile(std::string filepath, std::string data) {
  std::cout << "current dir: " << fs::current_path() << std::endl;

  std::cout << "writing file: " << filepath << std::endl;

  std::ofstream output(filepath.c_str());
  output << data;
}

void callback(wv::WebView &w, std::string &arg) {
  std::cout << arg << std::endl;
  auto j = json::parse(arg);
  auto type = j["type"].get<std::string>();
  std::cout << type << std::endl;

  auto id = format(j["id"].get<std::string>());

  if (type == "log") {
    std::cout << j["data"] << std::endl;
  } else if (type == "fs.readFile") {
    auto filepath = j["filepath"].get<std::string>();

    auto [contents, err] = readFile(filepath);

    w.eval(Str("window.external.cpp.emit('") + id + Str("', {data: '") +
           contents + Str("', err: '") + err + Str("'});"));
  } else if (type == "fs.writeFile") {
    auto filepath = j["filepath"].get<std::string>();
    auto data = j["data"].get<std::string>();
    writeFile(filepath, data);
    w.eval(Str("window.external.cpp.emit('") + id + Str("');"));
  } else {
    std::cout << j << std::endl;
  }
}

// Webviews
std::unordered_map<std::string, wv::WebView *> windows;

// App
JSValue log(JSContext *ctx, JSValueConst this_val, int argc,
            JSValueConst *argv) {
  for (int i = 0; i < argc; i++) {
    if (i != 0) std::cout << ' ';
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) return JS_EXCEPTION;
    std::cout << str;
    JS_FreeCString(ctx, str);
  }
  std::cout << std::endl;
  return JS_UNDEFINED;
}

// opts, ID
JSValue createWindow(JSContext *ctx, JSValueConst this_val, int argc,
                     JSValueConst *argv) {
  if (argc < 2) {
    std::cout << "n_createWindow: requires 2 arguments, got " << argc
              << std::endl;
    return JS_EXCEPTION;
  }

  // Get window ID
  std::string id = JS_ToCString(ctx, argv[1]);

  // Get window options
  auto opts = argv[0];

  // Get width
  int width = getIntPropDefault(ctx, opts, "width", 800);

  // Get height
  int height = getIntPropDefault(ctx, opts, "height", 600);

  // Get title
  auto title = getStrPropDefault(ctx, opts, "title", "Neutrino");

  // Background Color (rgba)
  int bgR = getIntPropDefault(ctx, opts, "_bgColorR", 255);
  int bgG = getIntPropDefault(ctx, opts, "_bgColorG", 255);
  int bgB = getIntPropDefault(ctx, opts, "_bgColorB", 255);
  int bgA = getIntPropDefault(ctx, opts, "_bgColorA", 255);

  wv::WebView *w = new wv::WebView{width, height, true, true, title};

  w->setCallback(callback);

  w->setBgColor(bgR, bgG, bgB, bgA);

  w->preEval(Str(R"inject(
    window.external.cpp = new function() {
      const onces = {};
      function once(eventName, callback) {
        if (onces[eventName] == null) {
            onces[eventName] = [];
        }
        onces[eventName].push(callback);
      }
      function emit(eventName, arg) {
        if (onces[eventName]) {
          onces[eventName].forEach(o => o(arg));
          delete onces[eventName];
        }
      }
      return {
        once,
        emit
      };
    }
  )inject"));

  if (w->init() == -1) {
    std::cout << "failed window init" << std::endl;
    return -1;
  }

  windows[id] = w;

  return 0;
}

JSValue navigate(JSContext *ctx, JSValueConst this_val, int argc,
                 JSValueConst *argv) {
  if (argc < 2) {
    std::cout << "n_navigate: requires 2 arguments, got " << argc << std::endl;
    return JS_EXCEPTION;
  }

  // Get window ID
  std::string id = JS_ToCString(ctx, argv[1]);

  // Get url
  wv::String url = format(std::string(JS_ToCString(ctx, argv[0])));

  Cout << Str("navigating to ") << url << std::endl;
  windows[id]->navigate(url);

  return JS_UNDEFINED;
}

JSValue quitApp(JSContext *ctx, JSValueConst this_val, int argv,
                JSValueConst *argc) {
  for (auto &it : windows) {
    wv::WebView *w = it.second;
    w->exit();
    delete w;
  }
  windows.clear();

  return JS_UNDEFINED;
}

bool evalString(JSContext *ctx, std::string js, const char *filename) {
  JSValue ret = JS_Eval(ctx, js.c_str(), js.length(), filename, 0);
  bool error = false;

  if (JS_IsException(ret)) {
    js_std_dump_error(ctx);
    error = true;
  }

  JS_FreeValue(ctx, ret);
  return error;
}

WEBVIEW_MAIN {
#ifdef WEBVIEW_WIN
  AllocConsole();
  FILE *out, *err;
  freopen_s(&out, "CONOUT$", "w", stdout);
  freopen_s(&err, "CONOUT$", "w", stderr);
  auto path = getPath();
#else
  auto path = getPath(argc, argv);
#endif

  if (path.empty()) {
    usage();
    return 0;
  }

  Cout << Str("Loading: ") << path << std::endl;

  // Get main file
  Cout << Str("Opening main: ") << path + Str("/main.js") << std::endl;
  std::ifstream mainFile(path + Str("/main.js"));
  if (!mainFile.is_open()) {
    std::cout << "Cannot open main file" << std::endl;
    return 1;
  }
  std::stringstream sstr;
  sstr << mainFile.rdbuf();
  std::string mainContents = sstr.str();
  mainFile.close();

  // Create QuickJS runtime and context
  JSRuntime *rt = JS_NewRuntime();
  if (!rt) {
    std::cout << "Could not allocate JS runtime" << std::endl;
    return 1;
  }

  JSContext *ctx = JS_NewContext(rt);
  if (!ctx) {
    std::cout << "Could not allocate JS context" << std::endl;
    return 1;
  }

  // Load global stuff (console.log)
  // js_std_add_helpers(ctx, 0, nullptr);

  // System modules
  // js_init_module_std(ctx, "std");
  // js_init_module_os(ctx, "os");

  // JS-C++ bindings
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global_obj, "n_log",
                    JS_NewCFunction(ctx, log, "n_log", 1));
  JS_SetPropertyStr(ctx, global_obj, "n_createWindow",
                    JS_NewCFunction(ctx, createWindow, "n_createWindow", 1));
  JS_SetPropertyStr(ctx, global_obj, "n_navigate",
                    JS_NewCFunction(ctx, navigate, "n_navigate", 1));
  JS_SetPropertyStr(ctx, global_obj, "n_quit",
                    JS_NewCFunction(ctx, quitApp, "n_quit", 1));
  JS_SetPropertyStr(ctx, global_obj, "__dirname",
                    JS_NewString(ctx, normalize(path).c_str()));

  // Execute main script
  std::cout << "running main" << std::endl;
  if (evalString(ctx, mainContents, "[main]")) {
    std::cout << "Main error" << std::endl;
    return 1;
  }

  // Emit ready event
  std::cout << "calling n_ready" << std::endl;
  if (evalString(ctx, "main.default.n_ready();", "[ready]")) {
    std::cout << "Ready error" << std::endl;
    return 1;
  }

  // Main loop
  std::cout << "entering main loop" << std::endl;
  while (!windows.empty()) {
    for (auto it = windows.begin(); it != windows.end();) {
      wv::WebView *w = it->second;
      if (w->run() != 0) {
        std::cout << "window closing" << std::endl;
        it = windows.erase(it);
        delete w;
      } else {
        ++it;
      }
    }
  }

  std::cout << "main loop ended" << std::endl;

  // Cleanup QuickJS
  js_std_free_handlers(rt);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
