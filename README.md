# neutrino

## Usage

```
npm install --save-dev neutrinojs
```

```
neutrinojs [path]
```

Path must be directory that contains

- index.html: renderer
- main.js: main script
  The main script will be injected into the renderer as a web worker
  Limitations:
- there must be a main window open at all times
