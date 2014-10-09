
var params = function() {
  return require( './config/' + 'params.json' );
};

console.log("Loading config file: " + "config.json" );
exports.config            = require('./config/config.json');

console.log("Loading globals file: " + "globals.json" );
exports.config.globals     = require( './config/globals.json' );

console.log("Loading parameters file: " + "params.json" );
exports.config.params     = require( './config/params.json' );
