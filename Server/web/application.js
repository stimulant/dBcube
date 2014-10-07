var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";
var clientStatusGood = false;
var paramSets;
var currentParam;

function startclient(clientidx) { socket.emit('startclient', clientidx); }
function killclient(clientidx) { socket.emit('killclient', clientidx); }
function restartclient(clientidx) { socket.emit('restartclient', clientidx); }

function startall() { socket.emit('startall'); }
function killall() { socket.emit('killall'); }
function restartall() { socket.emit('restartall'); }

function updatestatus(data)
{
  // recreate rows if our table row count is different
  if (data.clients.length != $('#clients tr').length)
  {
    $('#clients').empty();
    for (var i in data.clients ) {
      var tableHtml = '<tr>' +
      '<td id="client_host_' + i + '"/>' +
      '<td id="client_heatbeat_' + i + '"/>' +
      '<td id="client_fps_' + i + '"/>' +
      '<td id="client_isTop_' + i + '"/>' +
      '<td id="client_connectedTo_' + i + '"/>' +
      '<td><button id="startbtn" onClick="startclient(' + i + ')">start</button></td>' +
      '<td><button id="killbtn" onClick="killclient(' + i + ')">kill</button></td>' +
      '<td><button id="restartbtn" onClick="restartclient(' + i + ')">restart</button></td>' +
      '</tr>';
      $('#clients').append(tableHtml);
    }
  }

  // set initial params
  if (paramSets == undefined)
  {
    paramSets = data.config.params;
    currentParam = "defaults";
    updateParamCombo();
    updateParams();
  }
  
  // update clients
  var currentStatusGood = true;
  for (var i in data.clients )
  {
      //console.log(data.clients[i]);
      $('#client_host_' + i).html(data.clients[i].host);
      $('#client_heatbeat_' + i).html(data.clients[i].heartbeat_count.toPrecision(2));
      $('#client_fps_' + i).html(data.clients[i].fps.toPrecision(4));
      $('#client_isTop_' + i).html(data.clients[i].isTop ? "true" : "false");
      $('#client_connectedTo_' + i).html(data.clients[i].connectedTo);
      if (data.clients[i].heartbeat_count > 1.0)
        currentStatusGood = false;
  }
  
  // updata server status
  $('#serverHost').html("Server Host: " + data.config.server_host);
  $('#serverPort').html("Server Port: " + data.config.server_port);
  $('#oscserverHost').html("OSC Server Host: " + data.config.osc_server_host);
}

function changeParamSet(sel)
{
  currentParam = sel.value;
  updateParams();
  socket.emit('setcurrentparam', sel.value);
}

function deleteParamSet()
{
  if (currentParam == "defaults")
  {
    alert("You cannot delete the defaults param set!");
    return;
  }
  var r = confirm("Are you sure you want to delete param set " + currentParam + "?");
  if (r == true) 
  {
      delete paramSets[currentParam];
      currentParam = "defaults";
      socket.emit('setcurrentparam', currentParam);

      updateParamCombo();
      updateParams();
      socket.emit('updateparams', paramSets);
  }
}

function newParamSet()
{
  var paramSetName = prompt("Enter name of new Param Set", "New Param Set");
  if (paramSetName != null) 
  {
    paramSets[paramSetName] = jQuery.extend(true, {}, paramSets[currentParam]);
    currentParam = paramSetName;

    socket.emit('updateparams', paramSets);
    socket.emit('setcurrentparam', currentParam);

    updateParamCombo();
    $('#paramSetCombo')[0].selectedIndex = $('#paramSetCombo option').length-1;
  }
}

function updateParamCombo()
{
  // update combo
  if (Object.keys(paramSets).length != $('#paramSetCombo option').length)
  {
    $('#paramSetCombo').empty();
    for (var i in paramSets )
      $('#paramSetCombo').append("<option value='" + i + "'>" + i + "</option>");
  }
}

function updateParams()
{
  // update params form
  $('#params').empty();
  for (var i in paramSets[currentParam] ) {
    var paramHtml;

    if ( typeof(paramSets[currentParam][i]) == "boolean")
       paramHtml = '<input type="checkbox" id="' + i + 'Checkbox" ' + (paramSets[currentParam][i] ? 'checked/>' : '/>');
    else if ( typeof(paramSets[currentParam][i]) == "number")
       paramHtml = '<input id="' + i + 'Field" type="number" value="' + paramSets[currentParam][i] +'"/>';
    else if ( typeof(paramSets[currentParam][i]) == "string")
       paramHtml = '<input id="' + i + 'Field" value="' + paramSets[currentParam][i] +'"/>';

    var tableHtml = '<tr>' + '<td>' + i + '</td>' + '<td>' + paramHtml + '</td>' + '</tr>';
    //console.log("append" + tableHtml);
    $('#params').append(tableHtml);
  }
}

function checkParams()
{
  var paramChanged = false;
  for (var i in paramSets[currentParam])
  {
    if ( typeof(paramSets[currentParam][i]) == "boolean")
    {
      if (paramSets[currentParam][i] != $('#' + i + 'Checkbox').prop('checked'))
      {
        paramSets[currentParam][i] = $('#' + i + 'Checkbox').prop('checked');
        paramChanged = true;
      }
    }
    else if ( typeof(paramSets[currentParam][i]) == "number" && $.isNumeric($('#' + i + 'Field').val()) )
    {
      if (paramSets[currentParam][i] != parseFloat($('#' + i + 'Field').val()))
      {
        paramSets[currentParam][i] = parseFloat($('#' + i + 'Field').val());
        paramChanged = true;
      }
    }
    else if ( typeof(paramSets[currentParam][i]) == "string" && typeof($('#' + i + 'Field').val()) == "string" )
    {
        if (paramSets[currentParam][i] != $('#' + i + 'Field').val())
        {
          paramSets[currentParam][i] = $('#' + i + 'Field').val();
          paramChanged = true;
        }
    }
  }
  
  if (paramChanged)
    socket.emit('updateparams', paramSets);
}

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

// update clients list
socket.on('updateclients', function(data) { updatestatus(data); });

// send updated parameters on an interval
setInterval(
  function(){
    if (paramSets != undefined)
      checkParams(paramSets[currentParam]);
  }, 
1000.0/30.0);

