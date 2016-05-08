<?php
        require "../conn/pdo.php";
        require "../conn/conninfo.php";

        $sensors   = new conninfo($pdo);
        //$data_sensor_name = trim($_GET["sensor"]);
        $lastMinutes = trim($_GET["min"]);

	$arraySensors = array("A1");
	$arrayNum = 0;
	$sensorName = array();
	$jsonCategories = array();
	$jsonData = array();
        //if(strlen($data_sensor_name)>0) {
	foreach ($arraySensors as $data_sensor_name) {
                $nowtime = time();
                $startDate = $nowtime - (60*$lastMinutes);
                $endDate = $nowtime;
                $tmpList = $sensors->get_sensordata_range($data_sensor_name, $startDate, $endDate);
                $numX = count($tmpList)/10;
                $i=0;
		$jsonCategories[$arrayNum] = '[';
		$jsonData[$arrayNum] = '[';
                foreach ($tmpList as $row)
                {
                        if($i==0) $startTime = $row['datetime'];
                        $endTime = $row['datetime'];
                        $sensorName[$arrayNum] = $row['name'];
			if($i>0) {
				$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . ', ';
				$jsonData[$arrayNum] = $jsonData[$arrayNum] .  ', ';
			}
			$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "'".date("H:i", $row['datetime'])."'";
			$jsonData[$arrayNum] = $jsonData[$arrayNum] . $row['txtdata'];
                        $i++;
                }
		$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "]";
		$jsonData[$arrayNum] = $jsonData[$arrayNum] .  "]";
        }
        $dateTitle = date("Y-m-d H:i", $startTime) . "~" . date("Y-m-d H:i", $endTime);

?>

<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=UTF-8">
  <meta name="robots" content="noindex">
  <title><?php echo $dateTitle; ?></title>
  
  
  
  <script type='text/javascript' src='../hicharts/jquery-2.1.4.min.js'></script>
  
  
  <style type='text/css'>
    
  </style>
  




<script type='text/javascript'>//<![CDATA[

$(function () {

    $('#container').highcharts({
        title: {
            text: 'Monthly Average Temperature',
            x: -20 //center
        },
        subtitle: {
            text: 'Source: WorldClimate.com',
            x: -20
        },
        xAxis: {
            categories: <?php echo $jsonCategories[0]; ?>
        },
        yAxis: {
            title: {
                text: 'Temperature (°C)'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            valueSuffix: '°C'
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'middle',
            borderWidth: 0
        },
        series: [{
            name: '<?php echo $sensorName[0]; ?>',
            data: <?php echo $jsonData[0]; ?>
        }, {
            name: 'New York',
            data: [-0.2, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5]
        }, {
            name: 'Berlin',
            data: [-0.9, 0.6, 3.5, 8.4, 13.5, 17.0, 18.6, 17.9, 14.3, 9.0, 3.9, 1.0]
        }, {
            name: 'London',
            data: [3.9, 4.2, 5.7, 8.5, 11.9, 15.2, 17.0, 16.6, 14.2, 10.3, 6.6, 4.8]
        }]
    });
});
//]]> 

</script>

</head>
<body>
<?php echo $jsonCategories[0]; ?>
<?php echo "<br>".$jsonData[0]; ?>
<script src="../hicharts/js/highcharts.js"></script>
<script src="../hicharts/modules/exporting.js"></script>

<div id="container" style="min-width: 310px; height: 400px; margin: 0 auto"></div>

  
</body>

</html>
