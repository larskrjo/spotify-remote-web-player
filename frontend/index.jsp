<%@page contentType="text/html; charset=UTF-8" %>
<html> 
    <head>
        <title>Red Light House</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <link href="style/style.css" rel="stylesheet" type="text/css">
        <link rel="shortcut icon" href="images/redlights-icon.png">
        <script src="js/config.js"></script>
        <script src="js/jquery/jquery-2.1.0.min.js"></script>
        <script src="js/jquery/jquery-ui.js"></script>
        <script src="js/jquery/jquery.ui.position.js"></script>
        <script src="js/jquery/jquery.contextMenu.js"></script>
        <script src="js/contextMenu.js"></script>
        <script src="js/requests.js"></script>
        <script src="js/list.js"></script>
        <link href="style/jquery.contextMenu.css" rel="stylesheet" type="text/css">
    </head> 
    <body>
    <center>
    <div class="container">
    <div class="spotifyLogo"></div>
    <div class="spotifyTab" id="spotifyTab">
        <div class="searchTab" >
            <form method="get" action="#" class="searchForm" >
                <input id="searchField" type="search" name="q" size="32" maxlength="120" placeholder="Search for songs">
                <input type="submit" hidden="true">
            </form>
        </div>
        <div class="playlistTab">
        <ul>
            <li id="playlistLi" class="inUse"><a href="" class="playlistLink">Playlist</a></li>
        </ul>
        </div>
    </div>
    <div class="spotifyWindow" id="spotifyPlaylist" > 
        <table class="songtable" cellspacing="0" cellpadding="0" border="0" width="430">
        <tr>
        <td>
        <div style="width:430px; height:270px; overflow:auto;">
        <table id="playList" class="spList">
            <tbody id="playlistBody">
            </tbody>
        </table>
        </div>
        </td>
        </tr>
        </table>
        <p class="copyright" style="cursor:default;"><span style="-moz-transform: scaleX(-1); -o-transform: scaleX(-1); -webkit-transform: scaleX(-1); transform: scaleX(-1); display: inline-block;">©</span> Lars K. Johansen</p>
        <p class="trackadded" style="cursor:default;" id="trackremoved"></p>
    </div>
    <div class="spotifyWindow" hidden="true" id="spotifySearch" >
        <table id="didyoumeantable" cellspacing="0" class="spList" cellpadding="0" border="0" style="cursor:pointer;" width="430px" height="30px" hidden="true"><tr><td id="didyoumeantd" class="didyoumeantd"></td></tr></table>
        <table id="noresulttable" cellspacing="0" class="spList" cellpadding="0" border="0" style="cursor:default;" width="430px" height="30px" hidden="true"><tr><td id="noresulttd" class="noresulttd"><p>No search results</p></td></tr></table>
        <table id="didyoumeanyetother" cellspacing="0" class="songtable" cellpadding="0" border="0" width="430px">
        <tr>
        <td>
        <div id="didyoumeanother" style="width:430px; height:270px; overflow:auto;">
         <table id="searchList" class="spList">
            <tbody id="searchBody"></tbody>
        </table>
        </div>
        </td>
        </tr>
        </table>
        <p class="copyright" style="cursor:default;"><span style="-moz-transform: scaleX(-1); -o-transform: scaleX(-1); -webkit-transform: scaleX(-1); transform: scaleX(-1); display: inline-block;">©</span> Lars K. Johansen</p>
        <p class="trackadded" style="cursor:default;" id="trackadded"></p>
    </div>
    </div>
    </center>
    </body> 
</html>


