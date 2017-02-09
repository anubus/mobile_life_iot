Mobile Life IoT project forked from the Starter Kit reference project
====================

In a nutshell
-----------------
This project gathers data from the AT&T Starter Kit sensors as modified to 
support the Hackster.io Mobile Life IoT project and emits them to PubNub.

Prerequisites
-------------
The only thing required to run this project is an AT&T Developer account, but
it's meant to work with the AT&T Starter Kit (starterkit.att.com).  If you do
not have one of the AT&T Starter Kit hardware sets you can use the Virtual
Device tab in this project to mimic the hardware or even connect different
hardware.

Flow description
-----------------
Data: Endpoints and data flow for taking in sensor data.

Registration: A single use flow for registering a new device in M2X and 
creating data streams for the new device.  The Virtual Device flow contains a 
sequence which calls this flow, near the bottom.

Virtual Device: Mimics the Starter Kit hardware.

Trigger: Vestigal from the baseline Starter Kit example left in for future 
modification.  It is a low called by M2X on the Hot Temp and Cold Temp triggers 
firing.  Also contains the Command end point for setting up a command and a 
very simple polling mechanism whereby the device checks to see if any commands 
are waiting for it.

Additional information
----------------------
This project is free to use and modify.  Simply fork it to your own account.

Double-click on Comment nodes to see if there are any special instructions.


