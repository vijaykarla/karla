var request = require('request');

request.post({
  headers: {'content-type' : 'application/json'},
  url:     'http://localhost:25396/API/Values',
  body:    "\"{'resource':[{'lat':12.385479, 'lon':77.69093, 'speed':67.89879, 'etype':'G_PING', 'time':'2018-08-18 23:15:16'}]}\""
}, function(error, response, body){
  console.log(body);
});

//C# Code

 public void Post([FromBody]string value)
        {
            value = value.Replace("'", "\"");
            JsonValue jValue = JsonValue.Parse(value);
            JsonObject jObject = jValue as JsonObject;

            jValue = JsonValue.Parse(jObject["resource"][0].ToString());
            jObject = jValue as JsonObject;

            string latitude = jObject["lat"].ToString();
            string longitude = jObject["lon"].ToString();
                
        }