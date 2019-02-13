SELECT TOP 500 LocationId, DATEADD(mi, 330, EVENTTIME) EventTime, Latitude, Longitude, GroundSpeed, 
	CASE 
		WHEN EVENTTYPEID = 1 THEN 'G_PING'
		WHEN  EVENTTYPEID = 2 THEN 'REBOOT'
		ELSE  'OTHER' 
	END AS EventType
FROM Location WHERE TrackId = 44
ORDER BY LocationId DESC

http://valetron.in/api/v2/mysql/_table/log?api_key=7d1a4baac442ed875b1545af24f9f4312a1cfdb3fad0db096db91cf2869d17f2&fields=lat,lon,logid,devid,time,speed,server_time,pint&order=logid%20DESC&limit=100&filter=(devid=861311006352792)
