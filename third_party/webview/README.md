# webview

[![CMake CI](https://github.com/MichaelKim/webview/actions/workflows/ci.yaml/badge.svg)](https://github.com/MichaelKim/webview/actions/workflows/ci.yaml)

A tiny cross-platform webview library written in C++ using Edge on Windows (both EdgeHTML and Chromium), Webkit on MacOS, and WebkitGTK on Linux.

Inspired from zerge's [webview](https://github.com/webview/webview), this library was rewritten with several priorities:

- A more "C++"-like API
- Support for Microsoft Edge on Windows
- Replaced Objective-C runtime C code with actual Objective-C code

## Support

|            | Windows            | Windows            | MacOS                            | Linux                         |
| ---------- | ------------------ | ------------------ | -------------------------------- | ----------------------------- |
| Version    | Windows 10, v1809+ | Windows 7, 8.1, 10 | Tested on MacOS Mojave, Catalina | Tested on Ubuntu 18.04.02 LTS |
| Web Engine | EdgeHTML           | Chromium           | Webkit                           | WebKit                        |
| GUI        | Windows API        | Windows API        | Cocoa                            | GTK                           |

## Documentation

- [Build Steps](docs/build.md)
- [API Reference](docs/api.md)
- [Limitations](docs/limitations.md)

## Usage

```c++
#include "webview.h"

WEBVIEW_MAIN {
  // Create a 800 x 600 webview that shows Google
  wv::WebView w{800, 600, true, true, Str("Hello world!"), Str("http://google.com")};

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0);

  return 0;
}
```

Since Windows (WinAPI) uses `std::wstring`s, all string literals should be wrapped in the macro `Str(s)`.

The following URL schemes are supported:

- HTTP(S): `http://` and `https://`
- Local file: `file:///`, make sure to point to an `html` file
  - Not supported in Edge Legacy (see [Limitations](docs/limitations.md))
- Inline data: `data:text/html,<html>...</html>`

Check out example programs in the [`examples/`](examples/) directory in this repo.

Note: `WEBVIEW_MAIN` is a macro that resolves to the correct entry point:

```c++
#ifdef WEBVIEW_WIN
#define WEBVIEW_MAIN int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
#define WEBVIEW_MAIN int main(int, char **)
#endif
```

This is needed since Win32 GUI applications uses `WinMain` as the entry point rather than the standard `main`. You can write your own main for more control, but make sure to use `WinMain` if you need to support Windows.
