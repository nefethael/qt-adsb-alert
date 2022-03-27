# qt-adsb-alert

qt-adsb-alert fetches informations from ADS-B exchange website and sends a mail when interesting planes are near home 


# Releases

For Windows, you can download latest release (v1.0.0) here:
 * https://github.com/nefethael/qt-adsb-alert/releases/download/v1.0.0/qt-adsb-alert-v1.0.0.zip


# Settings

Settings are made in `setup.ini` file which resides in same folder as application.

* SMTP 

Only gmail smtp is supported, which will probably need a token from https://myaccount.google.com/apppasswords

In following example, commenting userSmtp disables email notification.

    ;userSmtp=Spotter Fan
    passSmtp=<gmail token>
    emailSmtp=adsb.spotter.fan@gmail.com

* Notify.run

You can also by notified by https://notify.run, you need to create a channel and then put the name channel in setup file (commenting disabled this notify system)

    ;notifyrun=lnW0LiQwmGKZNtheTkIm

* Pushbullet

You can be notified by https://www.pushbullet.com/, you need to create an "Access Token" in settings.

    ;pushbullet_user=adsb.spotter.fan@gmail.com
    ;pushbullet_token=o.ABC
    
* Telegram

You can be notified by Telegram, you need to:

1. create a channel
2. create a bot (send a message to @BotFather and follow instruction) and save token
3. invite bot in channel
4. retrieve chat id [see here](https://www.alphr.com/find-chat-id-telegram/)

Then complete setup.ini with corresponding informations:

    ;telegram_token=123:ABC
    ;telegram_chat=-666

* Home location

Set your latitude, longitude, altitude with corresponding values, for example for Bordeaux city: 

    home_lat=44.835
    home_lon=-0.536
    home_alt=0

* Tiles

ADSB Exchange loads information from different tiles, you can see which tiles are used for your location by inspecting website networking requests.
In this example, we load France South-West zone

    tiles=0016,6384,6505

# Scripting

qt-adsb-alert can send a mail when aircraft matches some conditions. These conditions are scriptable (javascript) and resides in `sendMail.mjs` file.
In the following example, we will alert if aircraft is closer than 50km or if it is closer than 100km and pointing toward home (+- 20°)

    export function sendAlert()
    {
        var distOk = (distanceToMe < 50000) || ((distanceToMe < 100000) && (gettingCloser < 20))
        return distOk                     // distance ok
    }
    
Exposed variables are 

- callsign
- hex
- typeCode
- typeDesc
- dbFlags
- altitude
- groundSpeed
- mach
- ias
- heading
- registration
- squawk
- distanceToMe
- gettingCloser

# Dependencies & Thanks

* OpenSSL for HTTPS (on windows you need libcrypto-1_1-x64.dll and libssl-1_1-x64.dll)
* https://github.com/bluetiger9/SmtpClient-for-Qt included at commit 3fa4a0fe5797
* https://github.com/wiedehopf/tar1090-db/blob/master/db/icao_aircraft_types2.js included at commit bb59b7144a
* https://github.com/wiedehopf/readsb for binary structures



