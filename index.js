var pse;
try {
  pse = require('./build/Release/pse');
} catch (e) {
  pse = require('./build/Debug/pse');
}

module.exports = pse;