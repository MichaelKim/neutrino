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
    callback(err, data);
  });
}

module.exports = {
  readFile
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
