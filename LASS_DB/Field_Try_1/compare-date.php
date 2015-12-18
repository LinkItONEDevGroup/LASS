<php
/*
 To access the page: http://nrl.iis.sinica.edu.tw/LASS/compare-date.php?device_1=FT1_001&date_1=2015-12-01&date_2=2015-12-02

*/
?>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>
    <script type="text/javascript" src="//thingspeak.com/highcharts-3.0.8.js"></script>
    <script type="text/javascript" src="//thingspeak.com/exporting.js"></script>

  <title>PM2.5 資訊比較(<?php echo $_GET['device_1']; ?>)</title>

  <style type="text/css">
  body { background-color: white; }
  #container { width: 100%; display: table; }
  #inner { vertical-align: top; display: table-cell; }
  #gauge_div { width: 450px; margin: 0 auto; }
  #chart-container { width: 800px; height: 250px; display: block; margin: 5px 15px 15px 0; overflow: hidden; }
</style>


  <script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type="text/javascript" src="http://maps.google.com/maps/api/js"></script>

<script type="text/javascript">
  var device_1 = '<?php echo $_GET['device_1']; ?>';
  var date_1 = '<?php echo $_GET['date_1']; ?>';
  var series_1_results = 1000;
  var series_1_color = '#ff0000';
  var device_2 = '<?php echo $_GET['device_1']; ?>';
  var date_2 = '<?php echo $_GET['date_2']; ?>';
  var series_2_results = 1000;
  var series_2_color = '#0000ff';

  // chart title
  var chart_title = '';
  // y axis title
  var y_axis_title = 'PM2.5';

  // user's timezone offset
  var my_offset = new Date().getTimezoneOffset();
  // chart variable
  var my_chart;

  // when the document is ready
  $(document).on('ready', function() {
    // add a blank chart
    addChart();
    // add the first series
    addSeries(device_1, date_1, series_1_results, series_1_color);
    addSeries(device_2, date_2, series_2_results, series_2_color);
  });

  // add the base chart
  function addChart() {
    // variable for the local date in milliseconds
    var localDate;

    // specify the chart options
    var chartOptions = {
      chart: {
        renderTo: 'chart-container',
        defaultSeriesType: 'line',
        backgroundColor: '#ffffff',
        events: { }
      },
      title: { text: chart_title },
      plotOptions: {
        series: {
          marker: { radius: 1 },
          animation: true,
          step: false,
          borderWidth: 0,
          turboThreshold: 0
        }
      },
      tooltip: {
        // reformat the tooltips so that local times are displayed
        formatter: function() {
              return this.series.name + ':<b>' + this.y + '</b><br>'+
                      'Time: '+ Highcharts.dateFormat('%I:%M %p', this.x);
        }
      },
      xAxis: {
        type: 'datetime',
        title: { enabled: true, text: 'Time of Day' },
        labels: {
            formatter: function() {
                return Highcharts.dateFormat('%H:%M', this.value);
            }
        },
        dateTimeLabelFormats : {
            hour: '%H',
            minute: '%H:%M',
            second: '%H:%M:%S' 
        }
      },
      yAxis: { title: { text: y_axis_title } },
      exporting: { enabled: false },
      legend: { enabled: false },
      credits: {
        text: '',
        href: '',

        style: { color: '#D62020' }
      }
    };

    // draw the chart
    my_chart = new Highcharts.Chart(chartOptions);
  }

  // add a series to the chart
  function addSeries(device, dates, results, color) {
    //var field_name = 'field' + field_number;
    var field_name = 's_d0';

    // get the data with a webservice call
    $.getJSON('http://nrl.iis.sinica.edu.tw/LASS/history-date.php', {device_id: escape(device), date:escape(dates)}, function(data) {

      // blank array for holding chart data
      var chart_data = [];
      var sma_num = 5;
      var sma = [];
      var k = 0;

      // iterate through each feed
      $.each(data.feeds, function() {
        var point = new Highcharts.Point();
        var value = this[field_name];
        //point.x = getChartDate(this.timestamp);
        var times = this.time.split(":");
        point.x = Date.UTC(2015,12,1,parseFloat(times[0]), parseFloat(times[1]));
        point.y = parseFloat(value);
        // if a numerical value exists add it
        if (!isNaN(parseInt(value))) {
                sma[k%sma_num] = point.y;
                k = k+1;
                if (k>sma_num){
                        point.y = 0;
                        for (var i=0;i<sma_num;i++){
                                point.y = point.y + sma[i];
                        }
                        point.y = point.y / sma_num;
                        chart_data.push(point);
                }
        }
      });

/*
      $.each(data.feeds, function() {
        var point = new Highcharts.Point();
        var value = this[field_name];
        //point.x = getChartDate(this.timestamp);
        var times = this.time.split(":");
        point.x = Date.UTC(2015,12,1,parseFloat(times[0]), parseFloat(times[1]));
        point.y = parseFloat(value);
        // if a numerical value exists add it
        if (!isNaN(parseInt(value))) { chart_data.push(point); }
      });
*/
      // add the chart data
      my_chart.addSeries({ data: chart_data, name: "PM2.5", color: color });
    });
  }

  // converts date format from JSON
  function getChartDate(d) {
    // offset in minutes is converted to milliseconds and subtracted so that chart's x-axis is correct
    return Date.parse(d) - (my_offset * 60000);
  }

</script>

  </head>

  <body>
    <center>
      <h1>PM2.5 資訊比較 (<?php echo $_GET['device_1']; ?>)：<font color="#ff0000"><?php echo $_GET['date_1']; ?></font> vs <font color="#0000ff"><?php echo $_GET['date_2']; ?></font> </h1>
    </center>

    <center>
    <div id="chart-container">
      <img alt="Ajax loader" src="//thingspeak.com/assets/loader-transparent.gif"/>
    </div>
    <hr>
    註：以上量測結果仍屬實驗階段，其正確性與代表性僅供參考，正確資料仍以環保署公佈為主。
    <br>
    Powered by <a href="https://www.facebook.com/groups/1607718702812067/">LASS</a> & <A href="https://sites.google.com/site/cclljj/NRL">IIS-NRL</a>
    </center>
  </body>
</html>
