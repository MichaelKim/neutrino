#include "duktape.h"
#include "json.hpp"
#include "webview.hpp"

#include <algorithm>               // std::replace
#include <experimental/filesystem> // fs::absolute, fs::exists
#include <fstream>                 // File read / write
#include <iostream>                // Debug output
#include <sstream>                 // File input to string
#include <unordered_map>           // std::vector

#ifdef WEBVIEW_WIN
#include <shellapi.h> // For CommandLineToArgvW
#endif

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

void usage() {
  std::cout << "Neutrino" << std::endl;
  std::cout << "Usage: neutrino [path]" << std::endl << std::endl;
}

wv::String format(std::string str) {
#ifdef WEBVIEW_WIN
  return std::wstring(str.begin(), str.end());
#else
  return str;
#endif
}

#if defined(WEBVIEW_WIN)
wv::String getPath() {
  if (fs::exists(fs::absolute("app") / "index.html")) {
    wv::String path = fs::absolute("app").wstring();
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
  }

  int argc;
  wchar_t **arglist = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc != 2) {
    return L"";
  }

  wv::String path(arglist[1]);
  // GetFullPathNameW
  if (path.find_first_of(L"http://") == 0 ||
      path.find_first_of(L"https://") == 0) {
    return path;
  }

  // Local file path
  return fs::absolute(path).wstring();
}
#else
wv::String getPath(int argc, char **argv) {
  if (fs::exists(fs::absolute("app") / "index.html")) {
    return fs::absolute("app").string();
  }

  if (argc != 2) {
    return "";
  }

  wv::String path(argv[1]);
  if (path.find_first_of("http://") == 0 ||
      path.find_first_of("https://") == 0) {
    return path;
  }

  // Local file path
  return fs::absolute(path).string();
}
#endif

void callback(wv::WebView &w, std::string &arg) {
  std::cout << arg << std::endl;
  json j = json::parse(arg);
  auto type = j["type"].get<std::string>();
  std::cout << type << std::endl;

  wv::String id = format(j["id"].get<std::string>());

  if (type == "log") {
    std::cout << j["data"] << std::endl;
  } else if (type == "fs.readFile") {
    try {
      auto filepath = j["filepath"].get<std::string>();

      std::cout << "current dir: " << fs::current_path() << std::endl;
      std::cout << "reading file: " << filepath << std::endl;

      std::ifstream input(filepath.c_str());
      if (!input.is_open()) {
        w.eval(Str("window.external.cpp.emit('") + id +
               Str("', {err: 'Missing file'});"));
        return;
      }
      std::stringstream sstr;
      sstr << input.rdbuf();

      wv::String contents = format(sstr.str());

      w.eval(Str("window.external.cpp.emit('") + id + Str("', {data: '") +
             contents + Str("'});"));
    } catch (json::exception &ex) {
      w.eval(Str("window.external.cpp.emit('") + id +
             Str("', {err: '") /* + ex.what() */ + Str("'});"));
    }
  } else if (type == "fs.writeFile") {
    try {
      auto filepath = j["filepath"].get<std::string>();
      auto data = j["data"].get<std::string>();

      std::cout << "current dir: " << fs::current_path() << std::endl;

      std::cout << "writing file: " << filepath << std::endl;

      std::ofstream output(filepath.c_str());
      output << data;

      w.eval(Str("window.external.cpp.emit('") + id + Str("');"));
    } catch (json::exception &ex) {
      w.eval(Str("window.external.cpp.emit('") + id +
             Str("', '") /* + ex.what() */ + Str("');"));
    }
  } else {
    std::cout << j << std::endl;
  }
}

// Webviews
std::unordered_map<std::string, wv::WebView *> windows;

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
  int width = duk_get_int_default(ctx, -1, 800);
  duk_pop(ctx);

  // Get height
  duk_get_prop_string(ctx, -1, "height");
  int height = duk_get_int_default(ctx, -1, 600);
  duk_pop(ctx);

  // Get title
  duk_get_prop_string(ctx, -1, "title");
  wv::String title = format(duk_get_string_default(ctx, -1, "Neutrino"));
  duk_pop(ctx);

  wv::WebView *w = new wv::WebView{width, height, true, true, title};

  w->setCallback(callback);

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

duk_ret_t navigate(duk_context *ctx) {
  // Get window ID
  std::string id = duk_to_string(ctx, -1);
  duk_pop(ctx);

  // Get url
  wv::String url = format(duk_to_string(ctx, -1));
  duk_pop(ctx);

  std::cout << "navigating to " << url << std::endl;
  windows[id]->navigate(url);

  return 0;
}

duk_ret_t quitApp(duk_context *ctx) {
  for (auto &it : windows) {
    wv::WebView *w = it.second;
    w->exit();
    delete w;
  }
  windows.clear();
}

WEBVIEW_MAIN {
#if defined(WEBVIEW_WIN)
  std::wstring path = getPath();
#else
  std::string path = getPath(argc, argv);
#endif

  if (path.length() == 0) {
    usage();
    return 0;
  }

#ifdef WEBVIEW_WIN
  std::wcout << L"Loading: " << path << std::endl;
#else
  std::cout << "Loading: " << path << std::endl;
#endif

  // Create Duktape heap and context
  duk_context *ctx = duk_create_heap_default();
  if (!ctx) {
    std::cout << "Could not create JS context" << std::endl;
    return 1;
  }

  // Get main file
  std::cout << "Opening main: " << path + "/main.js" << std::endl;
  std::ifstream mainFile(path + "/main.js");
  if (!mainFile.is_open()) {
    std::cout << "Cannot open main file" << std::endl;
    return 1;
  }
  std::stringstream sstr;
  sstr << mainFile.rdbuf();
  std::string mainContents = sstr.str();
  mainFile.close();

  // JS-C++ bindings
  duk_push_c_function(ctx, log, DUK_VARARGS);
  duk_put_global_string(ctx, "n_log");
  duk_push_c_function(ctx, createWindow, DUK_VARARGS);
  duk_put_global_string(ctx, "n_createWindow");
  duk_push_c_function(ctx, navigate, DUK_VARARGS);
  duk_put_global_string(ctx, "n_navigate");
  duk_push_c_function(ctx, quitApp, DUK_VARARGS);
  duk_put_global_string(ctx, "n_quit");
  duk_push_string(ctx, path.c_str());
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

  // Cleanup Duktape
  duk_destroy_heap(ctx);
  return 0;
}
