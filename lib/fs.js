function readFile(filepath, encoding, callback) {
  const id = Date.now().toString();

  window.external.invoke(
    JSON.stringify({
      type: 'fs.readFile',
      filepath,
      id
    })
  );

  window.external.cpp.once(id, ({ err, data }) => {
    if (err) {
      callback({ message: err }, null);
    } else {
      callback(null, data);
    }
  });
}

function writeFile(filepath, data, callback) {
  const id = Date.now().toString();

  window.external.invoke(
    JSON.stringify({
      type: 'fs.writeFile',
      filepath,
      id,
      data
    })
  );

  window.external.cpp.once(id, callback);
}

module.exports = {
  readFile,
  writeFile
};

/*

main listens to event (e.g. ready):
- app.on(eventName, callback)
  - Saves the callback
  - If there is a event from C++ (onmessage), fire all callbacks of that event

main requests for C++ API (e.g. fs):
- postMessage

renderer requests for C++ API (e.g. fs):
- window.external.invoke
  - Sends message to C++ containing API name and request ID
  - Once finished, C++ sends response to renderer

*/
