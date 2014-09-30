DBC
===

Decibel Cube

## Setup Kinect
1. Connect Kinect to USB 3.0 port (looks blue inside).
2. Install Kinect SDK from \\methlab\IT\Software\Windows\Dev\Kinect\2.0.
3. Run KinectService (only necessary with SDK releases before 1404).

## Setup Cinder
1. Clone Cinder from http://github.com/cinder.
2. Switch to the dev branch and build 64bit binaries inside of Visual Studio.
3. Map the CINDER_DIR environmental variable to your new Cinder folder.

## Running Cube on Sealab01
1. Make sure KinectService is running, if not start it.
2. Run DBCTest.exe int c:\stimulant\dbc\DBCTest.

## Running Kinect sample apps on Sealab01
1. Make sure KinectService is running, if not start it.
2. Run appropriate app in c:\stimulant\dbc\Kinect2.