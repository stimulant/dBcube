var config = require('./config.js');
var oscServer = require('./lib/oscServer.js');
var WebServer = require('./lib/webServer.js').WebServer;

config.config.paramsChanged = true;
config.config.currentParam = "defaults";
var webServer = new WebServer();
webServer.start(config.config, config.params);
oscServer.start(config.config, webServer);