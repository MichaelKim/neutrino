#!/usr/bin/env node

var proc = require('child_process');
var path = require('path');

var child = proc.spawn(
  path.join(__dirname, 'neutrino'),
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
