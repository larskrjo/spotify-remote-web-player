function playlistMenu() {
    $.contextMenu({
        selector: '.playlistSong', 
        callback: function(key, options) {
            var idstr = options.$trigger.attr('id');
            var id = idstr.substring(4);
            // Play song
            if(key == "play") {
                startSongFromMenu(id);
            }
            // Copy HTTP link
            else if(key == "copyhttp") {
                window.prompt("Copy to clipboard: Ctrl+C, Enter", playlistSongs[id][1]);
            }
            // Copy URL link
            else if(key == "copyuri") {
                window.prompt("Copy to clipboard: Ctrl+C, Enter", playlistSongs[id][2]);
            }
            // Copy URL link
            else if(key == "remove") {
                if(playlistSongs.length != 1) {
                    document.getElementById("trackremoved").innerHTML = "\""+$("#"+idstr).find("#songPlaylistName").html()+"\" removed!";
                    setTimeout(function(){document.getElementById("trackremoved").innerHTML = ""},2000);
                    $.getJSON( serveraddress+"/remove/"+id, function( data ) {});
                    populatePlaylistRequest();
                }
                else {
                    window.prompt("You cannot remove the last song in the playlist, the playback will stop!", "Ok!");
                }    
            }
        },
        items: {
            "play": {name: "Play", icon: "play"},
            "sep1": "---------",
            "copyhttp": {name: "Copy HTTP Link", icon: "copy"},
            "copyuri": {name: "Copy Spotify URI", icon: "copy"},      
            "sep2": "---------",
            "remove": {name: "Remove", icon: "remove"}
        }
    });
}

function searchlistMenu() {
    $.contextMenu({
        selector: '.searchlistSong', 
        callback: function(key, options) {
            var idstr = options.$trigger.attr('id');
            var id = idstr.substring(6);
            // Play song
            if(key == "add") {
                // Add to playlist
                $.getJSON( serveraddress+"/add/"+searchlistSongs[id][2], function( data ) {});
                // Set indicator that it has been added in the window
                document.getElementById("trackadded").innerHTML = "\""+$("#"+idstr).find("#songName").html()+"\" added!";
                var myVar = setTimeout(function(){document.getElementById("trackadded").innerHTML = ""},2000);

            }
            // Copy HTTP link
            else if(key == "copyhttp") {
                window.prompt("Copy to clipboard: Ctrl+C, Enter", searchlistSongs[id][1]);
            }
            // Copy URL link
            else if(key == "copyuri") {
                window.prompt("Copy to clipboard: Ctrl+C, Enter", searchlistSongs[id][2]);
            }
        },
        items: {
            "add": {name: "Add", icon: "add"},
            "sep1": "---------",
            "copyhttp": {name: "Copy HTTP Link", icon: "copy"},
            "copyuri": {name: "Copy Spotify URI", icon: "copy"},
        }
    });
}