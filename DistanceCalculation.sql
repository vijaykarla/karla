declare @startlat float  = 19.19515870000000
declare @startlng float  = 72.83493250000000

;WITH CTE AS
(
SELECT tr.TrackId, MIN(RouteName) AS RouteName, MIN(Radius) AS Radius, MIN(SQRT(
    POWER(111.2 * (Latitude - StartPointLatitude), 2) +
    POWER(111.2 * (StartPointLogitude - Longitude) * COS(Latitude / 57.3), 2))) AS Distance
FROM dbo.Location lc INNER JOIN TrackRoute tr ON lc.TrackId = tr.TrackId
WHERE lc.EventTime > DATEADD(HOUR, -4, GETUTCDATE())
GROUP BY tr.TrackId
)
SELECT TrackId, RouteName
FROM CTE WHERE CTE.Distance <= CTE.Radius

SELECT TOP 2 * from dbo.Location lc 
INNER JOIN dbo.TrackRoute tr ON lc.TrackId = tr.TrackId
GROUP BY lc.TrackId