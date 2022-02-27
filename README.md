# qt-adsb-alert

qt-adsb-alert fetches informations from ADS-B exchange website and sends a mail when interesting planes are near home 

WARNING
It's an ongoing project, use at your own risk ;)
For the moment, there are lots of static configuration stuff, it will be completed in near future


# dependancies

OpenSSL for HTTPS (on windows you need libcrypto-1_1-x64.dll and libssl-1_1-x64.dll)
https://github.com/bluetiger9/SmtpClient-for-Qt included at commit 3fa4a0fe5797
https://github.com/wiedehopf/tar1090-db/blob/master/db/icao_aircraft_types2.js included at commit bb59b7144a

# compilation

In craftmodel.cpp, set your email informations, you can also change your location (set to Bordeaux) and change globe_x.binCraft to corresponding area.
