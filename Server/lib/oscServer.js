var osc = require('node-osc');
var ce = require('cloneextend');

var oscClients = new Array();
var server;
var lastClientIndex = -1;
var lastClientUpdateElapsed = 0.0;
var audioLevel = 0.0;

function start(config, webServer)
{
	// create osc server
	server = new osc.Server(config.server_port, config.server_host);
	console.log("osc server started at " + config.server_host + ", port " + config.server_port);

	// create osc clients
	for (var i = 0; i < config.clients.length; i++)
	{
		config.clients[i].heartbeat_count = config.clients[i].fps = 0;
		oscClients.push(new osc.Client(config.clients[i].host, config.clients[i].port));
	}

	// receive messages
	server.on("message", function (msg, rinfo)
	{
		//console.log("Recv Message:" + msg[2][0]);

		if (msg[2][0] == "audioin")
		{
			//console.log("Recv Message:" + msg[2][1]);
			audioLevel = msg[2][1];
		}

		if (msg[2][0] == "connect")
		{
			console.log("client " + msg[2][1] + " connected...");
			config.paramsChanged = true;
		}

		if (msg[2][0] == "heartbeat")
		{
			var index = msg[2][1];
			var fps = msg[2][2];

			config.clients[index].heartbeat_count = 0;
			config.clients[index].connected = true;
			config.clients[index].fps = fps;
		}

		if (msg[2][0] == "mouseupdate")
		{
			var clientIdx = msg[2][1];

			// send messages to clients
			for (var i = 0; i < oscClients.length; i++)
			{
				if (i != clientIdx && 
					(config.clients[i].connectedTo == -1 || config.clients[i].connectedTo == clientIdx) &&
					!config.clients[i].isTop)
					oscClients[i].send('mouseupdate', msg[2][1], msg[2][2], msg[2][3], msg[2][4]);
			}
		}

		if (msg[2][0] == "bodyupdate")
		{
			var clientIdx = msg[2][1];
			var bodyCount = msg[2][2];
			lastClientIndex = clientIdx;
			lastClientUpdateElapsed = 0.0;

			// create message for bodies
			var message = new osc.Message("bodyupdate");
			message.append(clientIdx);
			message.append(bodyCount);
			var JointType_Count = 25;
			for (var b = 0; b < bodyCount; b++)
			{
				for (var j=0; j < JointType_Count; j++)
				{
					message.append(msg[2][ b * JointType_Count * 3 + j * 3 + 3]);
					message.append(msg[2][ b * JointType_Count * 3 + j * 3 + 4]);
					message.append(msg[2][ b * JointType_Count * 3 + j * 3 + 5]);
				}
			}

			// send messages to clients
			for (var i in oscClients)
			{
				if ((i != clientIdx && 
					(config.clients[i].connectedTo == -1 || config.clients[i].connectedTo == clientIdx) &&
					!config.clients[i].isTop) ||
					config.send_skeletons_to_all)
					oscClients[i].send(message);
			}
		}
	});

	// start update loop at 30fps
	var elapsed = 0;
	var paramsRotateElapsed = 0;
	var paramRotateIdx = 0;
	var updateSocket = 0;
	var audioDisabled = false;
	var audioDisabledTimeout = 0.0;
	var centerAttract = false;
	var centerAttractElapsed = 0.0;

	setInterval(function()
	{
		// disable audio after a period of quiet
		if (audioLevel >= 0.1)
		{
			if (audioDisabled)
			{
				audioDisabled = false;
				console.log("audio enabled");		
			}
			audioDisabledTimeout = 0.0;
		}
		else
		{
			audioDisabledTimeout += 1.0/30.0;
			if (!audioDisabled && config.audio_disable_when_quiet &&
				audioDisabledTimeout > config.audio_disable_timeout)
			{
				audioDisabled = true;
				console.log("audio disabled");
			}
		}

		// send config to clients
		for (var i = 0; i < oscClients.length; i++)
		{
			config.clients[i].heartbeat_count++;
			oscClients[i].send("config",
				server.host,
				server.port,
				i,
				oscClients.length,
				config.clients[i].isTop ? 1 : 0,
				config.clients[i].connectedTo
			);

			oscClients[i].send("audioUpdate", audioDisabled ? Math.abs(Math.sin(elapsed * 0.1)) : audioLevel);

			// send top messages to client
			if (config.clients[i].isTop)
				oscClients[i].send("topUpdate", lastClientIndex, lastClientUpdateElapsed);
		}

		if (config.center_attract)
		{
			//console.log("center attract update: " + centerAttractElapsed);
			centerAttractElapsed += 1.0/30.0;
			if (centerAttract && centerAttractElapsed > config.center_attract_duration)
			{
				console.log("turn off center attract");
				for (var i = 0; i < oscClients.length; i++)
					oscClients[i].send("centerAttract", 0);
				centerAttract = false;
				centerAttractElapsed = 0.0;
			}
			else if (!centerAttract && centerAttractElapsed > config.center_attract_timer)
			{
				console.log("turn on center attract");
				for (var i = 0; i < oscClients.length; i++)
					oscClients[i].send("centerAttract", 1);
				centerAttract = true;
				centerAttractElapsed = 0.0;
			}
		}

		if (config.param_rotate)
		{
			paramsRotateElapsed += 1.0/30.0;
			if (paramsRotateElapsed > config.param_rotate_time)
			{
				paramsRotateElapsed = 0.0;
				config.params[config.currentParam];
				paramRotateIdx = (paramRotateIdx + 1) % Object.keys(config.params).length;
				var idx = 0;
				for (var i in config.params)
				{
					if (idx == paramRotateIdx)
					{
						config.currentParam = i;
						config.paramsChanged = true;
					}
					idx++;
				}
			}
		}

		if (config.paramsChanged)
		{
			console.log("paramsChanged: " + config.currentParam);

			// make a merged copy of globals and current params
			var sendParams = ce.cloneextend(config.globals, config.params[config.currentParam]);

			for (var i = 0; i < oscClients.length; i++)
				oscClients[i].send("updateparams", JSON.stringify(sendParams));
			config.paramsChanged = false;
		}

		elapsed += 1.0/30.0;
		lastClientUpdateElapsed += 1.0/30.0;
	},1000.0/30.0);
}

exports.start          = start;