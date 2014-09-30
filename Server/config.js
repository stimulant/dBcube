
var params = function() {
  return require( './config/' + 'params.json' );
};

console.log("Loading config file: " + "defaults.json" );
exports.config            = require('./config/defaults.json');

console.log("Loading parameters file: " + "params.json" );
exports.config.params     = require( './config/params.json' );
