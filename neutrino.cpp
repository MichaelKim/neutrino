#include <filesystem>     // fs::absolute, fs::exists
#include <fstream>        // File read / write
#include <iostream>       // Debug output
#include <memory>         // std::unique_ptr
#include <sstream>        // File input to string
#include <unordered_map>  // std::vector

#include "duktape.h"
#include "json.hpp"
#include "webview.hpp"

#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
#include <shellapi.h>  // For CommandLineToArgvW
#endif

namespace fs = std::filesystem;
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
#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
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

void writeFile(const std::string& filepath, const std::string& data) {
  std::cout << "current dir: " << fs::current_path() << std::endl;

  std::cout << "writing file: " << filepath << std::endl;

  std::ofstream output(filepath.c_str());
  output << data;
}

void callback(wv::WebView &w, wv::String &arg) {
  Cout << arg << std::endl;
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
std::unordered_map<std::string, std::unique_ptr<wv::WebView>> windows;

// App
duk_ret_t log(duk_context *ctx) {
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  auto msg = duk_safe_to_string(ctx, -1);
  std::cout << msg << std::endl;
  return 0;
}

duk_ret_t createWindow(duk_context *ctx) {
  // Get window ID
  std::string id = duk_to_string(ctx, -1);
  duk_pop(ctx);

  // Get window options
  duk_to_object(ctx, -1);

  // Get width
  duk_get_prop_string(ctx, -1, "width");
  auto width = duk_get_int_default(ctx, -1, 800);
  duk_pop(ctx);

  // Get height
  duk_get_prop_string(ctx, -1, "height");
  auto height = duk_get_int_default(ctx, -1, 600);
  duk_pop(ctx);

  // Get title
  duk_get_prop_string(ctx, -1, "title");
  auto title = format(duk_get_string_default(ctx, -1, "Neutrino"));
  duk_pop(ctx);

  // Background Color (rgba)
  duk_get_prop_string(ctx, -1, "_bgColorR");
  auto bgR = duk_get_uint_default(ctx, -1, 255);
  duk_pop(ctx);
  duk_get_prop_string(ctx, -1, "_bgColorG");
  auto bgG = duk_get_uint_default(ctx, -1, 255);
  duk_pop(ctx);
  duk_get_prop_string(ctx, -1, "_bgColorB");
  auto bgB = duk_get_uint_default(ctx, -1, 255);
  duk_pop(ctx);
  duk_get_prop_string(ctx, -1, "_bgColorA");
  auto bgA = duk_get_uint_default(ctx, -1, 255);
  duk_pop(ctx);

  auto w = std::make_unique<wv::WebView>(width, height, true, true, title);

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

  windows[id] = std::move(w);

  return 0;
}

duk_ret_t navigate(duk_context *ctx) {
  // Get window ID
  std::string id = duk_to_string(ctx, -1);
  duk_pop(ctx);

  // Get url
  wv::String url = format(duk_to_string(ctx, -1));
  duk_pop(ctx);

  Cout << Str("navigating to ") << url << std::endl;
  windows[id]->navigate(url);

  return 0;
}

duk_ret_t quitApp(duk_context *ctx) {
  for (auto &it : windows) {
    auto &w = it.second;
    w->exit();
  }
  windows.clear();
  return 0;
}

WEBVIEW_MAIN {
#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
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
    Cout << L"Cannot open main file: " << path << std::endl;
    return 1;
  }
  std::stringstream sstr;
  sstr << mainFile.rdbuf();
  std::string mainContents = sstr.str();
  mainFile.close();

  // Create Duktape heap and context
  auto *ctx = duk_create_heap_default();
  if (!ctx) {
    std::cout << "Could not create JS context" << std::endl;
    return 1;
  }

  // JS-C++ bindings
  duk_push_c_function(ctx, log, DUK_VARARGS);
  duk_put_global_string(ctx, "n_log");
  duk_push_c_function(ctx, createWindow, DUK_VARARGS);
  duk_put_global_string(ctx, "n_createWindow");
  duk_push_c_function(ctx, navigate, DUK_VARARGS);
  duk_put_global_string(ctx, "n_navigate");
  duk_push_c_function(ctx, quitApp, DUK_VARARGS);
  duk_put_global_string(ctx, "n_quit");
  duk_push_string(ctx, normalize(path).c_str());
  duk_put_global_string(ctx, "__dirname");

  // Execute main script
  std::cout << "running main" << std::endl;
  if (duk_peval_string(ctx, mainContents.c_str())) {
    std::cout << "Main error: " << duk_safe_to_string(ctx, -1) << std::endl;
    return 1;
  }
  duk_pop(ctx);

  // Emit ready event
  std::cout << "calling n_ready" << std::endl;
  if (duk_peval_string(ctx, "main.default.n_ready();")) {
    printf("eval failed: %s\n", duk_safe_to_string(ctx, -1));
    return 1;
  }
  duk_pop(ctx);

  // Main loop
  std::cout << "entering main loop" << std::endl;
  while (!windows.empty()) {
    for (auto it = windows.begin(); it != windows.end();) {
      auto &w = it->second;
      if (w->run() != 0) {
        std::cout << "window closing" << std::endl;
        it = windows.erase(it);
      } else {
        ++it;
      }
    }
  }

  std::cout << "main loop ended" << std::endl;

  // Cleanup Duktape
  duk_destroy_heap(ctx);
  return 0;
}
