$(document).ready(function () {
    var ctx = document.getElementById("myChart").getContext('2d');
    var myChart = new Chart(ctx, {
        type: 'bar',
        data: {  
		   labels:[  
			  "2/20/2019",
			  "2/21/2019",
			  "2/23/2019",
			  "2/24/2019",
			  "2/25/2019"
		   ],
		   datasets:[  
			  {  
				 label:"KA 51 B 1229",
				 data:[  
					-21,
					17,
					43,
					15,
					-51
				 ],
				 extra:[
					'Mysore',
					'Bengaluru',
					'Mangalore',
					'Udupi',
					'Karkala'
				 ],
				 backgroundColor:"rgba(255, 99, 132, 0.2)",
				 borderColor:"rgba(255, 99, 132, 1)",
				 borderWidth:"1"
			  }
		   ]
		},
        options: {
            responsive: false,
            scales: {
                yAxes: [{
                    ticks: {
                        beginAtZero: true
                    }
                }],
				xAxes: [{
					type: 'time',
					time: {
						unit: 'day'
					}
				}]
            },
            tooltips: {
                callbacks: {					
                    label: function (tooltipItem, data) {
                        var label = data.datasets[tooltipItem.datasetIndex].label || '';
						var itemIndex = tooltipItem.index;
						var extraItem = data.datasets[tooltipItem.datasetIndex].extra[itemIndex];
                        if (label) {
                            var yVal = Math.round(tooltipItem.yLabel * 100) / 100;
                            if (yVal > 0)
                                label += ' delayed by ' + yVal + ' minutes at ' + extraItem;
                            else if(yVal < 0)
                                label += ' reached before ' + -1*yVal + ' minutes at ' + extraItem;
                            else
                                label += ' reached on time at ' + extraItem;
                        }
                        return label;
                    }
                }
            }
        }
    });
});