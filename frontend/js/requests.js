var playlistSongs = [];
var searchlistSongs = [];
var alternativeSearch;

function updateViews() {
    // Pretty listing of tracks
    $("table tr:nth-child(even)").addClass("even");
    $("table tr:nth-child(odd)").addClass("odd");
    $("tr.playlistSong").click(startSong);
    $("tr.searchlistSong").click(addSong);
    $("tr.playlistSong").dblclick(startSong);
    $("tr.searchlistSong").dblclick(addSong);
    
    $("#didyoumeantable").click(research);
    $("#didyoumeantable").dblclick(research);
    playlistMenu();
    searchlistMenu();
}

function research() {
    document.getElementById("searchField").value = alternativeSearch;
    makeSearchRequest();
}

function startSongFromMenu(numId) {
    $.getJSON( serveraddress+"/start/"+numId, function( data ) {});
    $("#play"+numId).addClass("active").siblings().removeClass("active");
}

function startSong() {
    var id = this.id;
    var numId = id.substring(4);
    $.getJSON( serveraddress+"/start/"+numId, function( data ) {});
    $("#"+id).addClass("active").siblings().removeClass("active");
}

function addSong() {
    var id = this.id;
    var numId = id.substring(6);
    $.getJSON( serveraddress+"/add/"+searchlistSongs[numId][2], function( data ) {});
    document.getElementById("trackadded").innerHTML = "\""+$("#"+this.id).find("#songName").html()+"\" added!";
    var myVar = setTimeout(function(){document.getElementById("trackadded").innerHTML = ""},2000);
}

function makeSearchRequest() {
    $.getJSON( serveraddress+"/search/"+document.getElementById("searchField").value, function( data ) {
        document.getElementById("playlistBody").innerHTML = '';
        document.getElementById("searchBody").innerHTML = '';
        playlistSongs = [];
        searchlistSongs = [];
        var suggestion = true;
        alternativeSearch = "";
        document.getElementById("didyoumeantd").innerHTML = "";
        document.getElementById('didyoumeantable').style.display = 'none';
        document.getElementById('noresulttable').style.display = 'none';
        document.getElementById('didyoumeanother').style.height = '270px';
        document.getElementById('didyoumeanyetother').style.height = '270px';
        $.each( data, function( key, val ) {
            if(suggestion) {
                if(val != "") {
                    alternativeSearch = val;
                    document.getElementById("didyoumeantd").innerHTML = "<p>Did you mean \""+val+"\"?</p>";
                    document.getElementById('didyoumeantable').style.display = 'block';
                    document.getElementById('didyoumeanother').style.height = '240px';
                    document.getElementById('didyoumeanyetother').style.height = '240px';
                }
                suggestion = false;
                return;
            }
            var info = [];
            info.push(val[3]);
            info.push(val[5]);
            info.push(val[4]);
            searchlistSongs.push(info);
            var seconds = (val[2]/1000)%60;
            var minutes = Math.floor((val[2]/1000)/60);
            if(seconds < 10)
                seconds = "0"+seconds;
            var time_string = minutes+":"+seconds;
            $("#searchBody").append("<tr class=\"searchlistSong\" style=\"cursor:pointer;\" id=\"search"+val[3]+"\"><td><div class=\"tdextra\" id=\"songName\" style=\"width:220px; cursor:pointer;\">"+val[0]+"</div></td><td><div class=\"tdextra\" style=\"width:130px;cursor:pointer;\">"+val[1]+"</div></td><td class=\"time\" style=\"cursor:pointer;\">"+time_string+"</td></tr>");
        });
        if(searchlistSongs.length == 0 && alternativeSearch == "") {
            document.getElementById('noresulttable').style.display = 'block';
            document.getElementById('didyoumeanother').style.height = '240px';
            document.getElementById('didyoumeanyetother').style.height = '240px';
        }
        updateViews();
    });
}

function populatePlaylistRequest() {
    $.getJSON( serveraddress+"/playlist", function( data ) {
        var items = [];
        document.getElementById("playlistBody").innerHTML = '';
        document.getElementById("searchBody").innerHTML = '';
        playlistSongs = [];
        searchlistSongs = [];
        $.each( data, function( key, value ) {
            $.each( value, function( key, val ) {
                var info = [];
                info.push(val[3]);
                info.push(val[6]);
                info.push(val[5]);
                playlistSongs.push(info);
                var seconds = (val[2]/1000)%60;
                var minutes = Math.floor((val[2]/1000)/60);
                if(seconds < 10)
                    seconds = "0"+seconds;
                var time_string = minutes+":"+seconds;
                $("#playlistBody").append("<tr class=\"playlistSong\" style=\"cursor:pointer;\" id=\"play"+val[3]+"\"><td><div class=\"tdextra\" id=\"songPlaylistName\" style=\"width:220px;cursor:pointer;\">"+val[0]+"</div></td><td><div class=\"tdextra\" style=\"width:130px;cursor:pointer;\">"+val[1]+"</div></td><td class=\"time\" style=\"cursor:pointer;\">"+time_string+"</td></tr>");
                if(val[4] == 1)
                    $("#play"+val[3]).addClass("active").siblings().removeClass("active");
            });
        });
        updateViews();
    });
}