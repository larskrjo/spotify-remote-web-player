Spotify Remote Web Player
=========================
Spotify Remote Web Player provides both a web player and a backend server to play spotify songs remotely from any browser using an arbitrary chosen playlist.

![alt text](https://github.com/larskrjo/spotify-remote-web-player/raw/master/frontend/images/screenshot.png "Spotify Remote Web Player")

Features
--------
* Spotify backend server that plays the songs from your playlist.
* Spotify frontend web client that can modify and control the playlist at the server.
* Complete setup and integration of server and client.

Info
--------
Throughout this guide, paths will always be relative to spotify-remote-web-player unless otherwise explicitly stated. When referring to either of the OSX or Linux folders, the annotation will be {OSX/Linux} and you should chose which folder that corresponds to the OS of your backend server.

Requirements
------------
* Both the frontend server and backend server must be connected to internet, and hence not behind a firewall. The ports used are TCP 80 and TCP backend port (whichever you choose) for frontend and backend respectively.

###Frontend:
* Web server that supports html, css and javascript. Apache or Tomcat should do it, there are plenty of tutorials on how to install one.

###Backend:
* A Linux or OSX server with a sound card, sound card drivers and sudo rights.
* Spotify premium user credentials must be available to use in configuration of the backend server.

Installation
------------
###Frontend
Copy the content of frontend into the root folder of your webserver, and that's it!

###Backend
#### Application Key
In order to start the backend server, it needs a spotify application key. Go to https://devaccount.spotify.com/my-account/keys/, and log in with your premium spotify user account. Request a key if you haven't done that. Download the binary application key, name it spotify_appkey.key and store it in backend.
#### Permissions
Make sure backend/{OSX/Linux}/start is executable with the user you are going to run it with. If not, run:
```
chmod 777 backend/{OSX/Linux}/start
```
####Libraries
In order to run the backend server, the following libraries must be compiled and installed.
#####Spotify
```
Linux:
cd libs/Linux
tar -zxvf libspotify-12.1.51-Linux-x86_64-release.tar.gz 
cd libspotify-12.1.51-Linux-x86_64-release
sudo make install prefix=/usr/local
OSX:
// Library does not need to be installed
```
#####Jansson
```
Linux & OSX:
cd libs/{OSX/Linux}
tar -zxvf jansson-2.6.tar.gz
cd jansson-2.6
./configure
make
sudo make install
```
#####APR
```
Linux & OSX:
cd libs/{OSX/Linux}
tar -zxvf apr-1.5.0.tar.gz
cd apr-1.5.0
./configure --prefix=/usr/local
make
sudo make install
```
#####Event
```
Linux & OSX:
cd libs/{OSX/Linux}
tar -zxvf libevent-2.0.21-stable.tar.gz
cd libevent-2.0.21-stable
./configure
make
sudo make install
```

Configuration
-------------
###Backend
####Add spotify credentials, choose playlist and specify hostname and port number.
```
// Execute the following lines as one command line.
echo -e "
#bin/bash \n
username=your_username \n
password=your_password \n
playlist_name=your_playlist_name \n
playlist_uri=your_playlist_uri \n
hostname=your_hostname \n
port=your_port
" >> backend/config
```
Replace your_username, your_password, your_playlist, your_hostname and your_port 
with your username, password, playlist, backend hostname and backend port respectively.

###Frontend
####Set address for both the frontend server and backend server
```
nano /frontend/js/config.js
// Change webaddress to the address of your frontend web server.
// Change serveraddress to the combination of hostname and port in
// the backend config file: hostname:port. Save.
```
Usage
-----
To run the backend server, execute:
```
cd backend/{OSX/Linux}
./start
```
To visit the frontend website, go to the website..!

Documentation
----------
###The API used by the browser is as follows:
------------------
####Get playlist
http://backendaddress/playlist
#####Return JSON format of the playlist
```
{
    tracks:
    [
        [
            track_name(String),
            track_artist(String),
            length_in_millisec(int),
            array_id(int),
            currently_playing(int),
            spotify_uri(String),
            http_link(String)
        ],
        ...
    ]
}
```
------------------
####Search for songs
http://backendaddress/search/song_name
#####Return JSON format of the playlist
```
[
    proposed_search_string(String),
    [
        track_name(String),
        track_artist(String),
        length_in_millisec(int),
        array_id(int),
        spotify_uri(String),
        http_link(String)
    ],
    ...
]
```
NOTE: Only the first element of the returned array is the proposed search string, the rest is the array of tracks.

---------------------
####Start/Change song
http://backendaddress/start/array_id

NOTE: It is only possible to start a song from the playlist, and hence the array_id is the one from getting the playlist [and not from search!].
#####Return empty JSON
```
{
}
```
------------
####Remove song
http://backendaddress/remove/array_id

NOTE: It is only possible to remove a song from the playlist, and hence the array_id is the one from getting the playlist [and not from search!].
#####Return empty JSON
```
{
}
```
------------
####Add song
http://backendaddress/add/spotify_uri

NOTE: spotify_uri for a song comes when searching for a song.
#####Return empty JSON
```
{
}
```

Changelog
---------

Version 1.0
* Nothing changed.


Credits
-------

Special thanks to the great contributors of the following project(s):

* Spotify API Server: https://github.com/liesen/spotify-api-server

License
-------

This project is licensed under Creative Commons Attribution-NonCommercial 4.0 International license (http://creativecommons.org/licenses/by-nc/4.0/).