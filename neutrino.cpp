// WebView callback example
// To run this example, host `index.html` locally at `localhost:8080`.
// Make sure to add a loopback exception if developing on Windows
// (check the README).

#include "json.hpp"
#include "webview.hpp"

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

#define PACKAGED_PATH "res/app/index.html"

void usage() {
  std::cout << "Neutrino" << std::endl;
  std::cout << "Usage: neutrino [path]" << std::endl << std::endl;
}

#if defined(WEBVIEW_WIN)
wv::String getPath() {
  if (fs::exists(PACKAGED_PATH)) {
    return Str(PACKAGED_PATH);
  }

  int argc;
  wchar_t **arglist = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc != 2) {
    return "";
  }

  wv::String path(arglist[1]);
  // GetFullPathNameW
  if (path.find_first_of("http://") == 0 ||
      path.find_first_of("https://") == 0) {
    return path;
  }

  // Local file path
  return "file://" + fs::absolute(path).string();
}
#else
wv::String getPath(int argc, char **argv) {
  if (fs::exists(PACKAGED_PATH)) {
    return Str(PACKAGED_PATH);
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
  return "file://" + fs::absolute(path).string();
}
#endif

void callback(wv::WebView &w, std::string &arg) {
  json j = json::parse(arg);

  if (j["type"] == "log") {
    std::cout << j["data"] << std::endl;
  } else if (j["type"] == "fs.readFile") {
    // TODO: add error checking
    std::string filepath = j["filepath"];
    std::string id = j["id"];

    std::cout << "current dir: " << fs::current_path() << std::endl;

    std::cout << "reading file: " << filepath << std::endl;

    std::ifstream input(filepath.c_str());
    std::stringstream sstr;
    sstr << input.rdbuf();

    std::string contents = sstr.str();

    std::cout << "contents: " << contents << std::endl;

    w.eval("window.external.cpp.emit('" + id + "', {err: null, data: '" +
           contents + "'});");
  } else {
    std::cout << j << std::endl;
  }
}

WEBVIEW_MAIN {
#if defined(WEBVIEW_WIN)
  wv::String path = getPath();
#else
  wv::String path = getPath(argc, argv);
#endif

  if (path.length() == 0) {
    usage();
    return 0;
  }

  std::cout << "Loading: " << path + "/index.html" << std::endl;

  wv::WebView w{
      800, 600, true, true, Str("WebView Callback"), path + "/index.html"};

  w.setCallback(callback);

  if (w.init() == -1) {
    return 1;
  }

  w.eval(R"inject(
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
  };
  window.external.neutrino = new Worker("main.js");
  window.external.neutrino.onmessage = e => window.external.invoke(e.data);
  console.log("worker injected");
  )inject");

  w.eval("window.external.neutrino.postMessage({type: 'emit', eventName: "
         "'ready'});");

  while (w.run() == 0)
    ;

  return 0;
}
