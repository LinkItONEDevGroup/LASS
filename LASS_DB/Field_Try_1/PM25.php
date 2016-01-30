<php
/*
 To access the page: http://nrl.iis.sinica.edu.tw/LASS/PM25.php?site=中央研究院&city=台北市&district=南港區&channel=61022&apikey=XZMEBWCDJQYPJ146

 where site is the unique name of this node, 
       city is the deployment city, 
       district is the deployment districy/village, 
       channel is the channel ID of the corresponding ThingSpeak channel, 
       apikey is the READ key of the corresponding ThingSpeak channel
*/
?>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>
    <script type="text/javascript" src="//thingspeak.com/highcharts-3.0.8.js"></script>
    <script type="text/javascript" src="//thingspeak.com/exporting.js"></script>

  <title>PM2.5 即時資訊 @ <?php echo $_GET['site']; ?></title>

  <style type="text/css">
  body { background-color: white; }
  #container { width: 100%; display: table; }
  #inner { vertical-align: top; display: table-cell; }
  #gauge_div { width: 450px; margin: 0 auto; }
  #chart-container { width: 800px; height: 250px; display: block; margin: 5px 15px 15px 0; overflow: hidden; }
</style>


  <script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type='text/javascript'>

  // set your channel id here
  var channel_id = <?php echo $_GET['channel']; ?>;
  // set your channel's read api key here if necessary
  var api_key = '<?php echo $_GET['apikey']; ?>';
  // maximum value for the gauge
  var max_gauge_value = 100;
  // name of the gauge
  var gauge_name = 'PM2.5';

  // global variables
  var chart, charts, data;

  // load the google gauge visualization
  google.load('visualization', '1', {packages:['gauge']});
  google.setOnLoadCallback(initChart);
  
  

  // display the data
  function displayData(point) {
    data.setValue(0, 0, gauge_name);
    data.setValue(0, 1, point);
    chart.draw(data, options);
  }

  // load the data
  function loadData() {
    // variable for the data point
    var p;

    // get the data from thingspeak
    $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json?api_key=' + api_key, function(data) {

      // get the data point
      p = data.field1;

      if (p<=35){
        $('#suggestion1').html("正常戶外活動。");
        $('#suggestion2').html("正常戶外活動。");
      } else if (p<=53){
        $('#suggestion1').html("正常戶外活動。");
        $('#suggestion2').html("有心臟、呼吸道及心血管疾病的成人與孩童感受到癥狀時，應考慮減少體力消耗，特別是減少戶外活動。");
      } else if (p<=70){
        $('#suggestion1').html("任何人如果有不適，如眼痛，咳嗽或喉嚨痛等，應該考慮減少戶外活動。");
        $('#suggestion2').html("<ol><li>有心臟、呼吸道及心血管疾病的成人與孩童，應減少體力消耗，特別是減少戶外活動。</li><li>老年人應減少體力消耗。</li><li>具有氣喘的人可能需增加使用吸入劑的頻率。</li></ol>");
      } else {
        $('#suggestion1').html("任何人如果有不適，如眼痛，咳嗽或喉嚨痛等，應減少體力消耗，特別是減少戶外活動。");
        $('#suggestion2').html("<ol><li>有心臟、呼吸道及心血管疾病的成人與孩童，以及老年人應避免體力消耗，特別是避免戶外活動。</li><li>具有氣喘的人可能需增加使用吸入劑的頻率。</li></ol>");
      }


      // if there is a data point display it
      if (p) {
        p = Math.round((p / max_gauge_value) * 100);
        displayData(p);
      }

      p = data.field2;
      $('#temperature').html(p);

      p = data.field3;
      $('#humidity').html(p);

    });
    
    $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/fields/1/last.json?timezone=Asia%2FTaipei', function(data) {

    var now = new Date(data.created_at)
    $('#lastupdate').html(now.toString());

    });
  }

  // initialize the chart
  function initChart() {

    data = new google.visualization.DataTable();
    data.addColumn('string', 'Label');
    data.addColumn('number', 'Value');
    data.addRows(1);

    chart = new google.visualization.Gauge(document.getElementById('gauge_div'));
    options = {width: 450, height: 450, redFrom: 53, redTo: 100, yellowFrom:35, yellowTo: 53, greenFrom:0, greenTo:35, minorTicks: 5};

    loadData();

    // load new data every 15 seconds
    setInterval('loadData()', 15000);
  }

</script>


<script type="text/javascript">
  var series_1_channel_id = <?php echo $_GET['channel']; ?>;
  var series_1_field_number = 1;
  var series_1_read_api_key = '';
  var series_1_results = 2000;
  var series_1_color = '#ff0000';

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
    addSeries(series_1_channel_id, series_1_field_number, series_1_read_api_key, series_1_results, series_1_color);
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
          marker: { radius: 3 },
          animation: true,
          step: false,
          borderWidth: 0,
          turboThreshold: 0
        }
      },
      tooltip: {
        // reformat the tooltips so that local times are displayed
        formatter: function() {
          var d = new Date(this.x + (my_offset*60000));
          var n = (this.point.name === undefined) ? '' : '<br>' + this.point.name;
          return this.series.name + ':<b>' + this.y + '</b>' + n + '<br>' + d.toDateString() + '<br>' + d.toTimeString().replace(/\(.*\)/, "");
        }
      },
      xAxis: {
        type: 'datetime',
        title: { text: 'Date' }
      },
      yAxis: { title: { text: y_axis_title } },
      exporting: { enabled: false },
      legend: { enabled: false },
      credits: {
//        text: 'ThingSpeak.com',
//        href: 'https://thingspeak.com/',
        text: '',
        href: '',

        style: { color: '#D62020' }
      }
    };

    // draw the chart
    my_chart = new Highcharts.Chart(chartOptions);
  }

  // add a series to the chart
  function addSeries(channel_id, field_number, api_key, results, color) {
    var field_name = 'field' + field_number;

    // get the data with a webservice call
    $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/fields/' + field_number + '.json?offset=0&round=2&results=' + results + '&api_key=' + api_key, function(data) {

      // blank array for holding chart data
      var chart_data = [];

      // iterate through each feed
      $.each(data.feeds, function() {
        var point = new Highcharts.Point();
        // set the proper values
        var value = this[field_name];
        point.x = getChartDate(this.created_at);
        point.y = parseFloat(value);
        // add location if possible
        if (this.location) { point.name = this.location; }
        // if a numerical value exists add it
        if (!isNaN(parseInt(value))) { chart_data.push(point); }
      });

      // add the chart data
      my_chart.addSeries({ data: chart_data, name: data.channel[field_name], color: color });
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
      <h1>PM2.5 即時資訊：<? echo $_GET['site']; ?>（<? echo $_GET['city'].$_GET['district']; ?>）</h1>
      <font size="+2">
      時間：<span id="lastupdate"> </span><br>
      溫度：</b><span id="temperature"></span>&#8451;；濕度：</b><span id="humidity"></span>%
      </font>
    </center>

    <center>
    <table border=0 width=800><tr>
    <td width=450>
      <div id="inner">
        <div id="gauge_div"></div>
      </div>
    </td>
    <td width=350>
    <ul>
      <li><b>針對一般民眾的活動建議：</b><span id="suggestion1"></span></li>
      <li><b>針對敏感性族群的活動建議：</b><span id="suggestion2"></span></li>
    </ul>
    </td>
    </tr></table>

    <div id="chart-container">
      <img alt="Ajax loader" src="//thingspeak.com/assets/loader-transparent.gif"/>
    </div>
    <hr>
    註：以上量測結果仍屬實驗階段，其正確性與代表性僅供參考，正確資料仍以環保署公佈為主。
    <br>
    Powered by <a href="https://www.facebook.com/groups/1607718702812067/">LASS</a> & <A href="https://sites.google.com/site/cclljj/NRL">IIS-NRL</a> & <a href="https://thingspeak.com">ThingSpeak.com</a>
    </center>
  </body>
</html>
