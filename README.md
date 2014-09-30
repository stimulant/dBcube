dBcube
===

Having a studio located in Seattle means that we get to attend and, this year, partner with the [Decibel](http://dbfestival.com) electronic music festival. Our contribution to the festival was a multi-user interactive experience that we built with a new hardware platform from Microsoft simply called [The Cube](http://blogs.microsoft.com/next/2014/09/24/microsoft-cube-decibel-dance-music/).

dBcube visualizes a collection of sensor data from the four Kinects placed in the sculpture, and also reacts in real-time to the beats of performers in the venue. Dancers and their friends can see themselves in a whole new way, and make interactive connections with dancers on the other sides of the sculpture that they might not know. When dancers arrive on opposite sides, their avatars become linked at the hands by virtual ribbons, turning their individual dance moves into flowing and twisting shapes, creating collaborative visual expressions as they move along with the music. dBcube cycles through a variety of virtual environments as the evening progresses.

## Setup Kinect2
* Connect Kinect2 to USB 3.0 port (looks blue inside).
* Install Kinect for Windows SDK 2.0 from [here](http://www.microsoft.com/en-us/download/details.aspx?id=43661).

## Setup Cinder
* Update Cinder via submodule by running 'git submodule update --init --recursive'.
* Switch to the dev branch and build 64bit binaries inside of Visual Studio.

## Setup and run the Server
* See [the Server documentation](Server/) for more details.

## Run the Cube client
* Run DBCClient.exe.
