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

function wrapConfig(config, filename) {
  if (!filename) return config;

  try {
    const filepath = path.join(cwd, filename);
    log('Looking for additional config at:', filepath);
    const configWrapper = require(filepath);
    const wrappedConfig = configWrapper(config);
    log('Loaded additional config!');
    return wrappedConfig;
  } catch (err) {
    error('Error while loading additional config!', err);
    process.exit();
  }
}

// Main
const mainConfig = wrapConfig(
  mainNeutrinoConfig({ path: path.join(cwd, 'dist/main') }, { mode }),
  argv.m
);

webpack(mainConfig, (err, stats) => {
  if (err) {
    error('Main process build failed!', err);
    return;
  }

  log('Done building main!');

  // Renderer
  const rendererConfig = wrapConfig(
    rendererNeutrinoConfig({ path: path.join(cwd, 'dist/renderer') }, { mode }),
    argv.r
  );

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
  const server = new WebpackDevServer(devServerOptions, compiler);

  let isFirstBuild = true;

  compiler.hooks.done.tap('neutrino-webpack', stats => {
    if (isFirstBuild) {
      log('Renderer server loaded!');

      // Start neutrino (neutrino.exe on Windows)
      const neutrinoExt = process.platform === 'win32' ? '.exe' : '';
      const pathToNeutrino = path.join(__dirname, 'neutrino' + neutrinoExt);
      const neutrino = spawn(pathToNeutrino, [mainConfig.output.path]);

      // If webpack closes
      compiler.hooks.watchClose.tap('neutrino-webpack', () => {
        log('Renderer server down, closing neutrino...');
        neutrino.kill();
      });

      // If neutrino closes
      neutrino.on('close', () => {
        log('Neutrino down, closing renderer server...');
        server.stop();
      });

      isFirstBuild = false;
    }
  });

  server.start();
});
