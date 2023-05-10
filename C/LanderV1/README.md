Lunar Lander derived from Javascript Source from http://moonlander.seb.ly/

Updates by Dave Henry for Nanocomp 10th May 2023
Install CMOC and LWTOOLS from 
http://sarrazip.com/dev/cmoc.html
http://www.lwtools.ca/
To LINUX
From the src folder run the following
cmoc --void-target --org=0000 --data=73b0 --limit=7A00 -i --intdir=int --srec lander_game.c nanocomp.c lander.c landscape.c
The srec will contain the code ready to download to the Nanocomp using the Load function of the Monitor.

This can be used with the LANDER2.F16 font available in Fonts folder with LANDERFONT.SREC
