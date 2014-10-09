# Install Node.js
Run installer from [here](http://nodejs.org/).

# Install Node modules
Run 'npm install' from the Server folder.

# Configure Server
Edit the config.json file in your Server folder with your favorite text editor (I use Sublime).  You will need to update these values to reflect the computers you will be running as clients and where certain files are located on your computer

| Value                                    | Description
|------------------------------------------|------------
|`pstools_dir`                         	   |directory where [PSTools](http://technet.microsoft.com/en-us/sysinternals/bb896649.aspx) live (the Server uses these to administrate the clients)
|`remote_client_dir`                       |remote directory on the clients where DBCClient.exe lives
|`remote_user`                       	   |remote user name to access client machines on the network
|`remote_pass`                       	   |remote password to access client machines on the network
|`server_name`                       	   |name of the server (can be anything)
|`server_host`                       	   |machine name of the server on the network
|`server_port`                       	   |port on which the server communicates with the clients
|`web_port`                       		   |port on which the server creates the admin web panel
|`client_restart`                          |true if server should try to restart clients if they don't respond
|`client_restart_time`                     |amount of time server should wait before restarting
|`center_attract`                          |true if server should tell clients to attract all particles to center periodically
|`center_attract_timer`                    |amount of time between attracting all particles to center
|`center_attract_duration`                 |duration of time all particles should be attracted to center
|`clients`                                 |an array of client configuration details for each side of cube
|`clients[].host`                          |the OSC host the client is listening on
|`clients[].port`                          |the OSC port the client is listening on
|`clients[].isTop`                         |true if the client is for the top side of cube
|`clients[].connectedTo`                   |index that this side is connected to

# Start Server
1. Run cmd shell and cd to Server folder.
2. Run 'node server.js' to start server.
  * You can also start the server with [node-mon](https://github.com/remy/nodemon) instead to do hot reloading.

# Use Admin Panel
The admin panel can be used to adminstrate the clients as well as to tweak the global state of the cube or add parameter sets for different looks for the cube.  To access the admin panel go to http://localhost:8000 (8000 is default but this will change if you have changed the web_port value above).  The top of the display will list the clients and their current status and provide buttons for stopping and starting the client application on those machines or restarting those machines (this functionality will require that [PSTools](http://technet.microsoft.com/en-us/sysinternals/bb896649.aspx) be correctly installed and configured).  Below this there are some commands for globally configuring the cube.  Finally at the bottom there are a number of parameters that can be set to configure how the cube looks and responds to music.

| Value                                    | Description
|------------------------------------------|------------
|`roomTextureSide`                         |the texture that should be used to texture the interal walls of the cube (this texture must be a PNG or JPEG and must live on the client machines in the Client\assets\textures folder)
|`skeletonAlpha`                           |alpha to fade out the skeleton particles
|`skeletonScale`                           |scale amount for the skeleton particles
|`emitterColorSide1`                       |color for the particles and ribbons for side 1 of the cube
|`emitterColorSide2`                       |color for the particles and ribbons for side 2 of the cube
|`emitterColorSide3`                       |color for the particles and ribbons for side 3 of the cube
|`emitterColorSide4`                       |color for the particles and ribbons for side 4 of the cube
|`ribbonStartSize`                         |size that the ribbons start out as (when they leave a skeleton's hand)
|`ribbonEndSize`                           |size that the ribbons ends at
|`ribbonLength`                            |length of ribbon
|`ribbonTwistSpeed`                        |how much the ribbon twists over time
|`ribbonConnectedWind`                     |the wind force that pulls ribbons into the cube (while the ribbon is connected to the other side)
|`ribbonConnectedGravity`                  |the gravity force that pulls ribbons down (while the ribbon is connected to the other side)
|`ribbonDisconnectedWind`                  |the wind force that pulls ribbons into the cube (while the ribbon is not connected to the other side)
|`ribbonDisconnectedGravity`               |the gravity force that pulls ribbons down (while the ribbon is not connected to the other side)
|`ribbonAlphaFade`                     	   |how much the ribbon fades out as it goes into a cube
|`topColor`                     		   |color of the top of the cube