require('winston-daily-rotate-file');

var winston = require('winston');
const logDir = 'peplog';

winston.loggers.add('logger', {
    transports: [
        new winston.transports.DailyRotateFile({
                filename: logDir + '/%DATE%.log',
                level: 'info',
                prepend: true,
                datePattern: 'YYYY-MM-DD-HH',
                maxFiles: '30d',
				format: winston.format.combine(
					winston.format.timestamp({
						format: 'HH:mm:ss'
					}),
					winston.format.json()
				)
            }
        )
    ]
});

var logger = winston.loggers.get('logger');
Object.defineProperty(exports, "LOG", {value: logger});