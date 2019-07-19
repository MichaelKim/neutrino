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

const package = require(path.resolve('./package.json'));
const { name } = package;

fs.removeSync(`./dist/${name}-unpacked`);
fs.ensureDirSync(`./dist/${name}-unpacked/app`);

const exePath = require.resolve('./neutrino');

fs.copyFileSync(exePath, `./dist/${name}-unpacked/${name}`);

fs.copySync('./dist/main', `./dist/${name}-unpacked/app`);
fs.copySync('./dist/renderer', `./dist/${name}-unpacked/app`);
