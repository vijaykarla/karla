var mqtt = require('mqtt');
const https = require('https');

var mqttoptions = {
    port: 15255,
    host: 'mqtt://m11.cloudmqtt.com',
    clientId: 'mqttjs_' + Math.random().toString(16).substr(2, 8),
    username: 'xxxxxxxxxxxxxxxxxx',
    password: 'xxxxxxxxxxxxxxxxxx',
    keepalive: 60,
    reconnectPeriod: 1000,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    encoding: 'utf8'
};
var client = mqtt.connect('mqtt://m11.cloudmqtt.com', mqttoptions);

client.on('message', function (topic, message) {	
	if (topic.trim() && message.trim()) {
		var topicParts = topic.split('/');
		if(topicParts.length > 1)
		{
			const trackId = topicParts[0];
			const trackToken = topicParts[1];
			const data = message.toString();
			
			const postoptions = {
			hostname: 'flaviocopes.com',
			port: 443,
			path: '/todos',
			method: 'POST',
			headers: {
				'Content-Type': 'application/json',
				'Content-Length': data.length
			  }
			};

			const req = https.request(postoptions, (res) => {
			  res.setEncoding('utf8');

			  res.on('data', (d) => {
				//process.stdout.write(d)
			  })
			});

			req.on('error', (error) => {
			  //console.error(error)
			})

			req.write(data)
			req.end()
			client.end()
		}
	}	
})