# neutrino

Build cross-platform desktop applications using your OS' native web engine.

Note: this project is still in its early stages. Feel free to tinker around with it, but don't use it in production.

Neutrino is a experimental lightweight alternative to the popular [Electron framework](https://github.com/electron/electron). It is capable of producing apps that are 100x smaller compared to Electron by leveraging the native web engine of the OS.

To render a webview, it uses my C++ [webview library](https://github.com/LenKagamine/webview).

## Usage

1. Clone this repo
2. Install Node dependencies: `npm install`
3. Build neutrino
   - `cmake -S . -B build` to generate build files
4. To use in another project, link it locally:
   - `npm install --save-dev /path/to/neutrino`

On Windows, the webview library requires specific build steps to compile. For full details, read the webview's [documentation](https://github.com/LenKagamine/webview/blob/master/docs/build.md).

First, you'll want to use Visual Studio. If using the EdgeHTML-based Edge Legacy browser, make sure the install the [C++/WinRT Visual Studio Extension](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt#visual-studio-support-for-cwinrt-xaml-the-vsix-extension-and-the-nuget-package).

Otherwise, to target the Chromium-based Edge, make sure to have the [proper version](https://docs.microsoft.com/en-us/microsoft-edge/webview2/gettingstarted/win32) of Edge installed (currently 82.0.488.0+), then install these NuGet packages:

- Microsoft.Windows.ImplementationLibrary
- Microsoft.Web.WebView2

## CLI

### Neutrino

Upon launching, Neutrino will first check for the file `./app/index.html`. If it exists, then it will try to load the app located there. This is intended for production builds.

Otherwise, Neutrino will accept a single CLI argument to locate the app:

```
Usage: neutrinojs [path]
```

The path must be a directory that contains at least two files:

- `main.js`: Entry point for the main process
- `index.html`: Web page for renderer process

### Neutrino Webpack

`neutrino-webpack` simplifies the integration with webpack and Neutrino by building the main and renderer processes using webpack. For development builds, the renderer process is run with `webpack-dev-server` and Neutrino is automatically opened.

```
Usage: neutrino-webpack [options]
```

Options are any combination of:

- `-p, --production`: Build a production version (default is dev build)
- `-m, --config-main [main config file]`: Custom webpack config file for main process
- `-r, --config-renderer [renderer config file]`: Custom webpack config file for renderer process
- `-h, --help`: Show help

Examples:

```sh
# Development build
neutrino-webpack -m ./webpack.main.config.js -r ./webpack.renderer.config.js

# Production build
neutrino-webpack -p -m ./webpack.main.config.js -r ./webpack.renderer.config.js
```

To modify and extend the webpack configurations, `neutrino-webpack` supports additional config files, specified using `-m` and `-r`. These files must export a function that accepts the provided config object as its only argument.

Within the function, feel free to make any modifications to the default config. The returned object will be used as the webpack config.

For example, here is an example of adding React to your build:

```js
module.exports = config => {
  // Add @babel/preset-react to the config
  config.rules.push({
    test: /\.jsx?$/,
    exclude: /node_modules/,
    use: [
      {
        loader: 'babel-loader',
        options: {
          presets: ['@babel/preset-react']
        }
      }
    ]
  });

  return config;
};
```

The default config files are located at `./webpack.main.config.js` and `./webpack.renderer.config.js`.

### Neutrino Builder

`neutrino-builder` takes the built main and renderer files (i.e. created by `neutrino-webpack`), and copies them into an unpacked app directory along with the `neutrino` binary. It will also place a copy of the `neutrino` executable, renamed to your package name.

```
Usage: neutrino-builder [options]
```

Options are any combination of:

- `-m, --main [path to built main files]`: default is `./dist/main`
- `-r, --renderer [path to built renderer files]`: default is `./dist/renderer`
- `-h, --help`: Show help

## Packaging

To package your app, use the `neutrino-builder` command to package all of your built app files into a directory (by default `./dist/${package-name}/app`.

To perform this manually, grab a copy of `neutrino` (`neutrino.exe` for Windows) and place it in a directory. Create an `app/` subdirectory, and place all of your app's files in it. At the very least, this must include

- `main.js`: Main process
- `index.html`: Renderer process

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

Neutrino uses three external libraries:

- [Duktape](https://duktape.org)
- [nlohmann's JSON library](https://github.com/nlohmann/json)
- [my own webview library](https://github.com/LenKagamine/webview)

The JSON library is fairly simple to compile, but my webview library is a bit more complicated. Check out the build steps [there](https://github.com/LenKagamine/webview#build).

### Windows

Use Visual Studio 2019 and CMake.

There is experimental support with `clang-cl`. Check out the [webview README](https://github.com/LenKagamine/webview#windows) under Windows.

#### TL;DR:

Install Windows 10 SDK, generate WinRT projection headers, install `clang-cl` (LLVM) and run this in Powershell:

```sh
clang-cl neutrino.cpp third_party/duktape/duktape.c /EHsc /TP /I "." /I "./third_party/webview/" /I "./third_party/duktape/" /I "./third_party/nlohmann/" /DWEBVIEW_WIN -Xclang -std=c++17 -Xclang -Wno-delete-non-virtual-dtor -o "lib/neutrino.exe" /link "WindowsApp.lib" "user32.lib" "kernel32.lib"
```

This will generate an executable at `./lib/neutrino.exe`.

### MacOS

Neutrino uses the experimental `std::filesystem` header, so you'll need a compiler with filesystem support.

The `clang` that comes with macOS Catalina includes it, but older versions may require either a linker flag (`-lc++fs`) or installing it directly from LLVM.

### Linux

Neutrino uses the experimental `std::filesystem` header, so you'll need a compiler with filesystem support. Depending on your compiler, this may require a linker flag (e.g. `lstdc++fs`).
