SELECT TOP 500 LocationId, DATEADD(mi, 330, EVENTTIME) EventTime, Latitude, Longitude, GroundSpeed, 
	CASE 
		WHEN EVENTTYPEID = 1 THEN 'G_PING'
		WHEN  EVENTTYPEID = 2 THEN 'REBOOT'
		ELSE  'OTHER' 
	END AS EventType
FROM Location WHERE TrackId = 44
ORDER BY LocationId DESC

--http://valetron.in/api/v2/mysql/_table/log?api_key=7d1a4baac442ed875b1545af24f9f4312a1cfdb3fad0db096db91cf2869d17f2&fields=lat,lon,logid,devid,time,speed,server_time,pint&order=logid%20DESC&limit=100&filter=(devid=861311006352792)


declare @Lat1 decimal(8,4), @Long1 decimal(8,4), @Lat2 decimal(8,4), @Long2 decimal(8,4)
declare @TrackId bigint

set @Lat1 = 12.97547900000000
set @Long1 = 77.59093000000000
set @Lat2 = 13.24134800000000
set @Long2 = 74.94722000000000

declare @TravelDistance decimal(28,10)

set @TravelDistance = ACOS(SIN(PI()*@Lat1/180.0)*SIN(PI()*@Lat2/180.0)+COS(PI()*@Lat1/180.0)*COS(PI()*@Lat2/180.0)*COS(PI()*@Long2/180.0-PI()*@Long1/180.0))*6371000

select @TravelDistance

DECLARE @GeoFences TABLE(
TrackId bigint,
GeoFenceId bigint,
LocationName nvarchar(356),
Type tinyint,
Latitude  decimal(18, 14),
Longitude  decimal(18, 14),
Radius int,
LastTriggered datetime,
TriggerGap int,
Point1Distance float,
Point2Distance float,
Direction bit,
Fire bit)

INSERT INTO @GeoFences (TrackId, GeoFenceId, LocationName, Type, Latitude, Longitude, Radius, LastTriggered, TriggerGap, Point1Distance, Point2Distance, Fire)
SELECT @TrackId, GeoFenceId, LocationName, Type, Latitude, Longitude, Radius, LastTriggered, TriggerGap, null, null, 0 FROM dbo.GeoFence 
WHERE TrackId = @TrackId AND Status = 1

IF (SELECT COUNT(*) FROM @GeoFences) > 0
BEGIN
	UPDATE @GeoFences SET Point1Distance = ACOS(SIN(PI()*@Lat1/180.0)*SIN(PI()*Latitude/180.0)+COS(PI()*@Lat1/180.0)*COS(PI()*Latitude/180.0)*COS(PI()*Longitude/180.0-PI()*@Long1/180.0))*6371000,
						  Point2Distance = ACOS(SIN(PI()*@Lat2/180.0)*SIN(PI()*Latitude/180.0)+COS(PI()*@Lat2/180.0)*COS(PI()*Latitude/180.0)*COS(PI()*Longitude/180.0-PI()*@Long2/180.0))*6371000
						  
	UPDATE @GeoFences SET Fire = 1,
	Direction = CASE WHEN Point2Distance < Point1Distance THEN 0 ELSE 1 END
	WHERE (((Type = 1 OR Type = 3) AND Point1Distance >= Radius AND Point2Distance < Radius) OR  ((Type = 2 OR Type = 3) AND Point1Distance <= Radius AND Point2Distance > Radius)) AND
	GETUTCDATE() > DATEADD(MINUTE, TriggerGap, LastTriggered)  

	IF (SELECT COUNT(*) FROM @GeoFences WHERE Fire = 1) > 0
	BEGIN
		INSERT INTO dbo.SmsQue (TrackId, MobileNumber, AppendText, ScheduleTime)
		SELECT tk.TrackId, MobileNumber, 'Vehicle ' + ISNULL(TrackName, TrackUrlName) + ' has ' + CASE WHEN Direction = 0 THEN 'reached ' ELSE 'departed from ' END + LocationName, GETUTCDATE()
		FROM @GeoFences gf INNER JOIN dbo.GeoFenceMobile gfm ON gf.GeoFenceId = gfm.GeoFenceId AND (gfm.Frequency = 0 OR gfm.Count < gfm.Frequency)
		INNER JOIN Track tk ON tk.TrackId = gf.TrackId
		WHERE gf.Fire = 1

		UPDATE dbo.GeoFence SET LastTriggered = GETUTCDATE() WHERE GeoFenceId IN (SELECT GeoFenceId FROM @GeoFences WHERE Fire = 1)
		UPDATE dbo.GeoFenceMobile SET Count = Count + 1, FetchTime = GETUTCDATE() WHERE GeoFenceId IN (SELECT GeoFenceId FROM @GeoFences WHERE Fire = 1) AND (Frequency = 0 OR Count < Frequency)
		
	END
		
END

