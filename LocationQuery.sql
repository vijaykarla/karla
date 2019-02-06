SELECT TOP 500 LocationId, DATEADD(mi, 330, EVENTTIME) EventTime, Latitude, Longitude, GroundSpeed, 
	CASE 
		WHEN EVENTTYPEID = 1 THEN 'G_PING'
		WHEN  EVENTTYPEID = 2 THEN 'REBOOT'
		ELSE  'OTHER' 
	END AS EventType
FROM Location WHERE TrackId = 44
ORDER BY LocationId DESC