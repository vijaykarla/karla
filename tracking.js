(function () {
    var locationTimeout;
    var distanceTimeout;
    var altitudeTimeout;
    var trackedtime = undefined;   
    var locations;
    var myMap;
    var odoDistance;
    var odoAltitude;
    var timeout = 16000;
    var pepDistance = 0.0;
    var totalDistance = 0.0;
    var pepAltitude = peptrack.Elevation;
    var prevAltitude = pepAltitude;
    var addAltitude = 0.0;
    var addErrors = 0;
	
	if (/*@cc_on!@*/false) { // check for Internet Explorer
			document.onfocusin = onFocus;			
		} else {
			window.onfocus = onFocus;			
		}
		
	function onFocus(){
		initialize(false);		
	}

    function initialize(refresh) {
        trackedtime = new Date(peptrack.EventTime);        
        locations = [{ "Latitude": peptrack.Latidue, "Longitude": peptrack.Longitude, "EventTime": peptrack.EventTime, "TrackId": peptrack.TrackId, "GroundSpeed": peptrack.GroundSpeed }];

        showtrackingtime();
        displayAddress();
        showSchedule();

        clearTimeout(locationTimeout);
		clearTimeout(distanceTimeout);
		clearTimeout(altitudeTimeout);
		
        getLocation();

		if(refresh)
		{
			myMap = new pepMap(peptrack.Latidue, peptrack.Longitude);
			myMap.initialize();		

			var ctx1 = document.getElementById('pep-distance').getContext('2d');
			odoDistance = new odometer(ctx1, {
				height: 25,
				digits: 4,
				decimals: 1,
				value: pepDistance,
				wobbleFactor: 0
			});

			var ctx2 = document.getElementById('pep-altitude').getContext('2d');
			odoAltitude = new odometer(ctx2, {
				height: 25,
				digits: 4,
				decimals: 1,
				value: pepAltitude,
				wobbleFactor: 0
			});
		}		
    }   

    function getLocation() {        
        var eventTime = new Date(peptrack.EventTime);
        fetchApiLocations(eventTime.toUTCString());
		locationTimeout = setTimeout(function () {
            fetchApiLocation();
        }, timeout);
    }

    function fetchApiLocations(trackDate) {
        var ajaxUrl = peptrack.DomainName + "api/" + peptrack.ApiVersion + "/data/locations/id/" + peptrack.TrackId + "?trackdate=" + trackDate + "&direction=back-1&token=" + peptrack.Token;
        if (peptrack.PrivateCode != '')
            ajaxUrl += "&code=" + peptrack.PrivateCode;
        jQuery.ajax({
            headers: {
                Accept: "application/json; charset=utf-8",
                "Content-Type": "application/json; charset=utf-8"
            },
            url: ajaxUrl,
            type: "GET",
            contentType: 'application/json; charset=utf-8',
            success: function (resultData) {
                if (resultData && resultData != undefined && resultData.length > 0) {
                    var eventTime = new Date(resultData[0].EventTime);
                    if ((trackedtime == undefined || trackedtime.getTime() != eventTime.getTime())) {
                        var loctn = resultData[0];
                        if (loctn && loctn.Latitude && loctn.Longitude) {
                            if (locations.length > 1)
                                locations.splice(1, 1);
                            locations.unshift(loctn);
                            showSpeed(false);
                        }
                    }
                }
            },
            error: function (jqXHR, textStatus, errorThrown) {
            },
            timeout: timeout
        });
    }
    
    function fetchApiLocation() {
        var ajaxUrl = peptrack.DomainName + "api/" + peptrack.ApiVersion + "/data/location/id/" + peptrack.TrackId + "?token=" + peptrack.Token;
        if (peptrack.PrivateCode != '')
            ajaxUrl += "&code=" + peptrack.PrivateCode;
        jQuery.ajax({
            headers: {
                Accept: "application/json; charset=utf-8",
                "Content-Type": "application/json; charset=utf-8"
            },
            url: ajaxUrl,
            type: "GET",
            contentType: 'application/json; charset=utf-8',
            success: function (resultData) {
                if (resultData && resultData.Latitude && resultData.Longitude) {
                    var result = [parseFloat(resultData.Latitude), parseFloat(resultData.Longitude)];
                    var eventTime = new Date(resultData.EventTime);
                    if ((trackedtime == undefined || trackedtime.getTime() != eventTime.getTime())) {
						
						peptrack.Latidue = resultData.Latitude;
						peptrack.Longitude = resultData.Longitude;
						peptrack.EventTime = resultData.EventTime;						
						
                        if (locations.length > 1)
                            locations.splice(0, 1);
                        locations.push(resultData);
                        showDistance(totalDistance, true);
                        showSpeed(true);
                        showDistance(totalDistance, false);
                        showtrackingtime();
                        addErrors = 0;
                        displayAddress();
                        showSchedule();

                        var currentAltitude = parseFloat(resultData.Elevation);
                        if (prevAltitude == 0.0 && currentAltitude != 0)
                            showAltitude(currentAltitude + 15, true)
                        else
                            showAltitude(prevAltitude, true);

                        if (currentAltitude != 0) {
							peptrack.Elevation = currentAltitude;
                            prevAltitude = currentAltitude;
                            showAltitude(currentAltitude, false);
                        }
                        else {
							peptrack.Elevation = addAltitude;
                            prevAltitude = addAltitude;
                            showAltitude(addAltitude, false);
                        }
                        clearTimeout(pepMap.markerTimeout);
                        myMap.transition(result);
                    }
                    trackedtime = eventTime;
                }
            },
            error: function (jqXHR, textStatus, errorThrown) {
               
            },
            timeout: timeout
        });
		locationTimeout = setTimeout(function () {
            fetchApiLocation();
        }, timeout);
    }    
   
    function showtrackingtime() {
        var dateoptions = {
            weekday: "long", year: "numeric", month: "short",
            day: "numeric", hour: "2-digit", minute: "2-digit"
        };
        $("#pep-date").html(trackedtime.toLocaleString("en-us", dateoptions));
    }

    function displayAddress() {
        if (locations.length > 0) {
            currentlocation = locations[locations.length - 1];            
            var lat = currentlocation.Latitude;
            var long = currentlocation.Longitude;
            if (locations.length > 1) {
                var prevlocation = locations[locations.length - 2];
                var prevlat = prevlocation.Latitude;
                var prevlong = prevlocation.Longitude;

                var distance = geodistance(locations[locations.length - 2], locations[locations.length - 1]);
                if (distance < 0.01)
                    return;
            }
            var ajaxUrl = peptrack.DomainName + "api/" + peptrack.ApiVersion + "/data/address?latitude=" + lat + "&longitude=" + long + "&token=" + peptrack.Token;
            jQuery.ajax({
                headers: {
                    Accept: "application/json; charset=utf-8",
                    "Content-Type": "application/json; charset=utf-8"
                },
                url: ajaxUrl,
                type: "GET",
                contentType: 'application/json; charset=utf-8',
                success: function (resultData) {
                    if (resultData) {
                        addErrors = 0;
                        var address = resultData.Address;
                        var province = resultData.Province;
                        var country = resultData.Country;
                        var distance = parseFloat(resultData.Distance);
                        if (resultData.Elevation && resultData.Elevation != null)
                            addAltitude = parseFloat(resultData.Elevation);
                        var strDist = '';
                        if (distance > 1000) {
                            var fltDist = distance / 1000;
                            fltDist = fltDist.toFixed(1);
                            strDist = fltDist + " Km";
                        }
                        else
                            strDist = Math.round(distance) + " metres";
                        var strAdd = strDist + ' away from<br/>';
                        strAdd += address + '<br/>';
                        if (province != undefined)
                            strAdd += province + '<br/>';
                        if (country != undefined)
                            strAdd += country;

                        var address = '<span style="font-weight: bold; color:red">' + strDist + ' away from:</span>';
                        address += '<br/><span style="color:#6600cc;">' + resultData.Address;
                        if (resultData.Province != 'Unknown')
                            address += '<br/>' + resultData.Province;
                        if (resultData.PostalCode != undefined)
                            address += ' ' + resultData.PostalCode;
                        if (resultData.Country != 'Unknown')
                            address += '<br/>' + resultData.Country;
                        address += '</span>';
                        $('#pep-address').show();
                        $('#pep-address').html(address);
                    }
                    else if (addErrors < 5) {
                        addErrors++;
                        displayAddress();
                    }
                },
                error: function (jqXHR, textStatus, errorThrown) {
                    if (addErrors < 5) {
                        addErrors++;
                        displayAddress();
                    }
                },
                timeout: timeout
            });
        }
    }

    function scheduleClick(anchor) {
        var divAnchor = $(".selected");
        divAnchor.removeClass("selected");
        anchor.addClass("selected");
        showSchedule();
        return false;
    }

    var firstDistance = -1;
    function showSchedule() {
        if (locations.length > 0) {
            var boardDiv = $(".selected");
            var bordLatlong = boardDiv.attr("data-location");
            var bordLatlongs = bordLatlong.split(",");
            var boardLocation = { "Latitude": parseFloat(bordLatlongs[0]), "Longitude": parseFloat(bordLatlongs[1])};
            var distance = geodistance(locations[locations.length - 1], boardLocation);
            if (firstDistance < 0) {
                firstDistance = distance;
            }
            if (firstDistance > 0) {
                var leftPercentage = 100 - Math.round((distance / firstDistance) * 100);
                if (leftPercentage <= 100 && leftPercentage > 0) {
                    $(".divpointer").css({ left: leftPercentage.toString() + '%' });
                }
            }
            else if (firstDistance == 0) {
                $(".divpointer").css({ left: '99%' });
            }
            else if (firstDistance < 0) {
                $(".divpointer").css({ left: '0%' });
            }
            initDist = distance;
            var strDist = '';
            if (distance > 1) {
                var fltDist = distance.toFixed(1);
                strDist = fltDist + " Km";
            }
            else
                strDist = Math.round(distance*1000) + " metres";
            strDist += " away from ";
            var boardName = boardDiv.html();
            $("#spndistance").html(strDist);
            $("#spnpoit").html(boardName);
        }
    }

    var speedgauge = null;
    function geodistance(location1, location2) {
        var R = 6371; // km (change this constant to get miles)
        var dLat = (location2.Latitude - location1.Latitude) * Math.PI / 180;
        var dLon = (location2.Longitude - location1.Longitude) * Math.PI / 180;
        var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
            Math.cos(location1.Latitude * Math.PI / 180) * Math.cos(location2.Latitude * Math.PI / 180) *
            Math.sin(dLon / 2) * Math.sin(dLon / 2);
        var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
        var d = R * c;
        return d;
    }

    function showSpeed(addDistance) {
        if (speedgauge == null)
            speedgauge = new Gauge("pep-speed", { 'mode': 'needle', 'range': { 'min': 0, 'max': 180 } });
        var speed = 0;
        if (locations.length > 1) {
            speed = Math.round(parseFloat(locations[locations.length - 1].GroundSpeed));
            var currentlocation;
            var prevlocation;
            var distance;           
            currentlocation = locations[locations.length - 1];
            prevlocation = locations[locations.length - 2];           
            var distance = geodistance(currentlocation, prevlocation);
            if (distance >= 0) {
                var time2 = new Date(currentlocation.EventTime);
                var time1 = new Date(prevlocation.EventTime);
                var diffMs = time2 - time1;
                if (diffMs > 0) {
                    var diffHrs = diffMs / 1000 / 60 / 60;
                    if(speed == 0)
                        speed = Math.round(distance / diffHrs);                    
                    if (addDistance) {
                        if (diffHrs < 8)
                            totalDistance += distance;
                        else
                            totalDistance = 0;
                    }
                }
            }
        }
        else if (locations.length > 0) 
            speed = Math.round(parseFloat(locations[locations.length - 1].GroundSpeed));

        if (!speed)
            speed = 0;
		peptrack.GroundSpeed = speed;
        if (speed >= 0 && speed <= 180)
            speedgauge.draw(speed);
        else if (speed < 0)
            speedgauge.draw(0);
        else if (speed > 180)
            speedgauge.draw(180);
    }

    function showDistance(newDistance, isHard) {
        if (isHard) {
            clearTimeout(distanceTimeout);
            pepDistance = newDistance;
        }
        else
            pepDistance += 0.005;
        if (pepDistance > newDistance)
            pepDistance = newDistance;
        odoDistance.setValue(pepDistance);
        if (pepDistance < newDistance)
            distanceTimeout = setTimeout(function () { showDistance(newDistance, isHard); }, 100);
    }

    function resetDistance() {
        clearTimeout(distanceTimeout);
        totalDistance = 0.0;
        pepDistance = 0.0;
        showDistance(0, true);
    }

    var altcount = 0;
    function showAltitude(newAltitude, isHard) {
        if (isHard) {
            altcount = 0;
            clearTimeout(altitudeTimeout);
            pepAltitude = newAltitude;
        }
        else if (newAltitude > pepAltitude)
            pepAltitude += 0.01;
        else if (newAltitude < pepAltitude)
            pepAltitude -= 0.01;
        odoAltitude.setValue(pepAltitude);
        var diffAltitude = newAltitude - pepAltitude;
        if (diffAltitude < -0.02 || diffAltitude > 0.02) {
            altcount++;
            if (altcount > 1500) {
                altcount = 0;
                clearTimeout(altitudeTimeout);
                pepAltitude = newAltitude;
            }
            altitudeTimeout = setTimeout(function () { showAltitude(newAltitude, isHard); }, 10);
        }
    }

    function showPoints() {
        $('#overlay').show();
        $('#pointpopup').show();
        var selected = $('#pointpopup .selected');
        selected.parent().parent().scrollTop(selected[0].offsetTop - 145);
    }
    function hidePoints() {
        $('#pointpopup').hide();
        $('#overlay').hide();
    }

    function bindPointClicks() {
        $("#pointpopup").on("click", "a", function (e) {
            var clickedOne = $(e.target);
            scheduleClick(clickedOne);
        });
    }

    $(document).ready(function () {
        $("#divhouse").click(function () {
            //showPoints();
        });
        $("#spnpoit").click(function () {
            //showPoints();
        });
        $("#spndistance").click(function () {
            //showPoints();
        });
        $("#overlay").click(function () {
            hidePoints();
        });
        $("#imgreset").click(function () {
            resetDistance();
        });
        bindPointClicks();
        initialize(true);    
    });
})();
