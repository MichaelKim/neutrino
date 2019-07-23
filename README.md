# neutrino

Build cross-platform desktop applications using your OS' native web engine.

Note: this project is still in its early stages. Feel free to tinker around with it, but don't use it in production.

Neutrino is a experimental lightweight alternative to the popular [Electron framework](https://github.com/electron/electron). It is capable of producing apps that are 100x smaller compared to Electron by leveraging the native web engine of the OS.

To render a webview, it uses my C++ [webview library](https://github.com/LenKagamine/webview).

## Usage

First, clone or download this repo, then link it locally:

```
npm install --save-dev ./path/to/neutrino
```

## CLI

Upon launching, Neutrino will first check for the file `./app/index.html`. If it exists, then it will try to load the app located there. This is intended for production builds.

Otherwise, Neutrino will accept a single CLI argument to locate the app:

```
neutrinojs [path]
```

Path must be directory that contains

- index.html: renderer
- main.js: main script

## Packaging

To package your app, all of the app files must be located in `./dist/dev/`. At the very least, this includes `index.html` and `main.js`.

Once it's ready, run the command `neutrino-builder`. This will copy the contents of both directories and place them in `./dist/${your-package-name}/app`. It will also place a copy of the `neutrino` executable into `./dist/${your-package-name}`, renamed to your package name.

## Supported APIs

Currently, Neutrino is highly limited and only supports the following Node / Electron APIs:

- app
  - on
    - 'ready'
- BrowserWindow
- fs
  - readFile
  - writeFile

## Build

### Windows

Use Visual Studio 2019 and CMake.

```
clang-cl neutrino.cpp /EHsc /I "." /I "./third_party/webview/" /I "./third_party/nlohmann/" /DWEBVIEW_WIN -Xclang -std=c++17 -Xclang -Wno-delete-non-virtual-dtor -o "lib/neutrino.exe" /link "WindowsApp.lib" "user32.lib" "kernel32.lib"
```

### MacOS

### Linux

Neutrino uses the experimental `std::filesystem` header, so you'll need a compiler with filesystem support. It also uses two external libraries:

- [nlohmann's JSON library](https://github.com/nlohmann/json)
- [my own webview library](https://github.com/LenKagamine/webview)

The JSON library is fairly simple to compile, but my webview library is a bit more complicated. Check out the build steps [there](https://github.com/LenKagamine/webview#build).
