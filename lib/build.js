#!/usr/bin/env node

/*
Build process:
- Make folder "dist"
- Make folder "dist/${package name}"
- Copy neutrino executable into "dist/${package name}"
- Make folder "dist/${package name}/app"
- Copy everything in "dist/dev" to "dist/${package name}/app"
*/

const fs = require('fs-extra');
const path = require('path');

const package = require(path.resolve('./package.json'));
const { name } = package;

fs.removeSync(`./dist/${name}-unpacked`);
fs.ensureDirSync(`./dist/${name}-unpacked/app`);

const execExt = process.platform === 'win32' ? '.exe' : '';
const exePath = require.resolve(`./neutrino${execExt}`);

fs.copyFileSync(exePath, `./dist/${name}-unpacked/${name}${execExt}`);

fs.copySync('./dist/dev', `./dist/${name}-unpacked/app`);
