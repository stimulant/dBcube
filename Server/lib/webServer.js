var fs = require('fs');
var express = require('express');
var webapp = express();
var http = require('http');
var exec = require('child_process').exec;

var WebServer = function(){};

WebServer.prototype.updateClientHeartbeat = function(client)
{
	client.heartbeat_count += 1.0/30.0;

	// restart client if we haven't heard from them in a while
	if (this.config.client_restart &&
		client.heartbeat_count > this.config.client_restart_time)
	{
		// otherwise just try restarting the client app
		console.log("Restarting client " + client.host);
		this.killclient(client);
		this.startclient(client);
		client.heartbeat_count = 0;
		client.restart_count += 1;
	}
};
WebServer.prototype.startclient = function(idx)
{
	var cmdstr = this.config.pstools_dir + '\\psexec.exe /accepteula -i -w ' + this.config.remote_client_dir + ' -d \\\\' + this.clients[idx].host;
	if (this.config.remote_user != "")
		cmdstr += ' -u ' + this.config.remote_user
	if (this.config.remote_pass != "")
		cmdstr += ' -p ' + this.config.remote_pass;
	cmdstr += ' ' + this.config.remote_client_dir + '\\DBCClient.exe';
	console.log("START CLIENT CMD: " + cmdstr);
	exec(cmdstr);
};
WebServer.prototype.killclient = function(idx)
{
	var cmdstr = this.config.pstools_dir + '\\pskill.exe /accepteula \\\\' + this.clients[idx].host;
	if (this.config.remote_user != "")
		cmdstr += ' -u ' + this.config.remote_user
	if (this.config.remote_pass != "")
		cmdstr += ' -p ' + this.config.remote_pass;
	cmdstr += ' ' + 'DBCClient.exe';
	console.log("KILL CLIENT CMD: " + cmdstr);
	exec(cmdstr);
};

WebServer.prototype.startall = function() { for (var c in this.clients) this.startclient(c); };
WebServer.prototype.killall = function() { for (var c in this.clients) this.killclient(c); };

WebServer.prototype.update = function()
{
	this.elapsed += 1.0/30.0;
	
	if (this.updateSocket++ > 10)
	{
		var data = {};
		data.clients = this.clients;
		data.config = this.config;
		this.io.sockets.emit('updateclients', data);
		updateSocket = 0;
	}

	// update heartbeat for clients
	for (var i in this.clients )
		this.updateClientHeartbeat(this.clients[i]);
};

WebServer.prototype.connect = function (socket)
{
	// control individual clients
	socket.on('startclient', this.startclient.bind(this) );
	socket.on('killclient', this.killclient.bind(this) );

	// control all clients
	socket.on('startall', this.startall.bind(this));
	socket.on('killall', this.killall.bind(this));

	// user disconnects
	socket.on('disconnect', function(){});

	// params
	socket.on('updateparams', this.updateparams.bind(this));
	socket.on('setcurrentparam', this.setcurrentparam.bind(this));
};

WebServer.prototype.updateparams = function(newparams)
{
	if (newparams !== null)
	{
		this.config.params = newparams;
		this.config.paramsChanged = true;
		
		// save params
		//console.log("saveparams");
		var outputFilename = 'config/params.json';
		fs.writeFile(outputFilename, JSON.stringify(this.config.params, null, 4), function(err) {
			if(err) {
				console.log(err);
			} else {
				console.log("JSON saved to " + outputFilename);
			}
		});
	}
};

WebServer.prototype.setcurrentparam = function(currentParam)
{
	if (currentParam !== null)
	{
		this.config.currentParam = currentParam;
		this.config.paramsChanged = true;
	}
};

WebServer.prototype.start = function (config)
{
	this.config = config;
	this.clients = config.clients;
	
	this.webserver = http.createServer(webapp);
	this.io = require('socket.io').listen(this.webserver);
	this.io.set('log level', 1); // reduce logging
	console.log( "Socket.io listening on port", config.web_port );
	this.webserver.listen( config.web_port );

	// routing
	webapp.get('/', function (req, res) {
		fs.readFile(__dirname + '/../web/servers.html', function read(err, data) {
			if (err) {
				console.log(err);
			}
			else {
				// replace server name and url in file
				var msg = data.toString();
				msg = msg.replace("%server_url%", "http://" + req.headers.host);
				msg = msg.replace("%server_name%", config.server_name);
				res.send(msg);
			}
		});
	});
	webapp.use(express.static(__dirname + "/../web"));

	// user connects
	this.io.sockets.on('connection', this.connect.bind(this));

	// start update loop
	this.elapsed = 0;
	this.updateSocket = 0;
	setInterval(this.update.bind(this), 1000.0/30.0);
};

exports.WebServer          = WebServer;