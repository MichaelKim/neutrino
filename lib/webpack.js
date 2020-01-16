#!/usr/bin/env node

const path = require('path');
const { spawn } = require('child_process');
const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');
const mainNeutrinoConfig = require('../webpack.main.config');
const rendererNeutrinoConfig = require('../webpack.renderer.config');
const argv = require('yargs')
  .usage('Usage: $0 [options]')
  .alias('h', 'help')
  .alias('v', 'version')
  .alias('p', 'production')
  .describe('p', 'Build a production version (default is dev build)')
  .alias('m', 'config-main')
  .describe('m', 'Custom webpack config file for main process')
  .alias('r', 'config-renderer')
  .describe('r', 'Custom webpack config file for renderer process').argv;

const mode = argv.p ? 'production' : 'development';
const cwd = process.cwd();

function log(...args) {
  console.log('[neutrino-webpack]:', ...args);
}

function error(...args) {
  console.error('[neutrino-webpack:', ...args);
}

// Main
let mainConfig = mainNeutrinoConfig(
  { path: path.join(cwd, 'dist/main') },
  { mode }
);
if (argv.m) {
  try {
    const mainPath = path.join(cwd, argv.m);
    log('Looking for additional main config at:', mainPath);
    const configWrapper = require(mainPath);
    mainConfig = configWrapper(mainConfig);
    log('Loaded additional main config!');
  } catch (err) {
    error('Error while loading additional main config!', err);
    process.exit();
  }
}

webpack(mainConfig, (err, stats) => {
  if (err) {
    error('Main process build failed!', err);
    return;
  }

  log('Done building main!');

  // Renderer
  let rendererConfig = rendererNeutrinoConfig(
    { path: path.join(cwd, 'dist/renderer') },
    { mode }
  );
  if (argv.r) {
    try {
      const rendererPath = path.join(cwd, argv.r);
      log('Looking for additional renderer config at:', rendererPath);
      const configWrapper = require(rendererPath);
      rendererConfig = configWrapper(rendererConfig);
      log('Loaded additional renderer config!');
    } catch (err) {
      error('Error while loading additional renderer config!', err);
      process.exit();
    }
  }

  const compiler = webpack(rendererConfig);

  if (argv.p) {
    // Production: just build
    compiler.run((err, stats) => {
      if (err) {
        error('Renderer process build failed!', err);
      } else {
        log('Done building renderer!');
      }
    });

    return;
  }

  // Development: run dev server and Neutrino
  const devServerOptions = rendererConfig.devServer;
  const server = new WebpackDevServer(compiler, devServerOptions);

  let isFirstBuild = true;

  server.listen(devServerOptions.port, devServerOptions.host, () => {
    server.compiler.hooks.done.tap('neutrino-webpack', stats => {
      if (isFirstBuild) {
        log('Renderer server loaded!');

        // Start neutrino (neutrino.exe on Windows)
        const neutrinoExt = process.platform === 'win32' ? '.exe' : '';
        const pathToNeutrino = path.join(__dirname, 'neutrino' + neutrinoExt);
        const neutrino = spawn(pathToNeutrino, [mainConfig.output.path]);

        // If webpack closes
        server.compiler.hooks.watchClose.tap('neutrino-webpack', () => {
          log('Renderer server down, closing neutrino...');
          neutrino.kill();
        });

        // If neutrino closes
        neutrino.on('close', () => {
          log('Neutrino down, closing renderer server...');
          server.close();
        });

        isFirstBuild = false;
      }
    });
  });
});
