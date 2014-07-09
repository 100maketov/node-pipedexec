
var pse;
pse = require('./build/Release/pse');

console.log(pse.exec('python', './sleep.py'));
