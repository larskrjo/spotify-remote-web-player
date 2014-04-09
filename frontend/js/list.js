function switchToSearch() {
    $("#playlistLi").removeClass("inUse").addClass("notInUse") ;
    $("#searchLi").removeClass("notInUse").addClass("inUse");
    document.getElementById('spotifyPlaylist').style.display = 'none';
    document.getElementById('spotifySearch').style.display = 'block';
    makeSearchRequest();
    return false;
}

function switchToPlaylist() {
    populatePlaylistRequest();
    $("#playlistLi").removeClass("notInUse").addClass("inUse");
    $("#searchLi").removeClass("inUse").addClass("notInUse");
    document.getElementById('spotifyPlaylist').style.display = 'block';
    document.getElementById('spotifySearch').style.display = 'none';
    $("#searchField").val('');
    return false;
}

function listeners() {
    // React on search form and tab
    $("form.searchForm").on('submit', switchToSearch);
    $("a.playlistLink").click(switchToPlaylist);
    populatePlaylistRequest();
    updateViews();
}

$(document).ready(listeners);