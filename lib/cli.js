#!/usr/bin/env node

const proc = require('child_process');
const path = require('path');

const execName = process.platform === 'win32' ? 'neutrino.exe' : 'neutrino';

const child = proc.spawn(
  path.join(__dirname, execName),
  process.argv.slice(2),
  {
    stdio: 'inherit',
    windowsHide: false
  }
);
child.on('close', function(code) {
  process.exit(code);
});

const handleTerminationSignal = function(signal) {
  process.on(signal, function signalHandler() {
    if (!child.killed) {
      child.kill(signal);
    }
  });
};

handleTerminationSignal('SIGINT');
handleTerminationSignal('SIGTERM');
