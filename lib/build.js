#!/usr/bin/env node

/*
Build process:
- Make folder "dist"
- Make folder "dist/${package name}"
- Copy neutrino executable into "dist/${package name}"
- Make folder "dist/${package name}/app"
- Copy everything in "dist/main" to "dist/${package name}/app"
- Copy everything in "dist/renderer" to "dist/${package name}/app"
*/

const fs = require('fs-extra');
const path = require('path');
const argv = require('yargs')
  .usage('Usage: $0 [options]')
  .alias('h', 'help')
  .alias('v', 'version')
  .alias('m', 'main')
  .describe('m', 'Path to main build folder (default: ./dist/main)')
  .alias('r', 'renderer')
  .describe('r', 'Path to renderer build folder (default: ./dist/renderer)')
  .argv;

const mainPath = argv.m || './dist/main';
const rendererPath = argv.r || './dist/renderer';

const { name } = require(path.resolve('./package.json'));

fs.removeSync(`./dist/${name}-unpacked`);
fs.ensureDirSync(`./dist/${name}-unpacked/app`);

const execExt = process.platform === 'win32' ? '.exe' : '';
const exePath = require.resolve(`./neutrino${execExt}`);

fs.copyFileSync(exePath, `./dist/${name}-unpacked/${name}${execExt}`);

fs.copySync(mainPath, `./dist/${name}-unpacked/app`);
fs.copySync(rendererPath, `./dist/${name}-unpacked/app`);
