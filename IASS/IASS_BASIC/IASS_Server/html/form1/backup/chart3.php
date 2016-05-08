<?php

        require "../conn/pdo.php";
        require "../conn/conninfo.php";

        $sensors   = new conninfo($pdo);
        $data_sensor_name = trim($_GET["sensor"]);
	$lastMinutes = trim($_GET["min"]);

        include "../libchart/libchart/classes/libchart.php";

        $chart = new LineChart(1200,600);

        $dataSet = new XYDataSet();

	if(strlen($data_sensor_name)>0) {
		$nowtime = time();
		$startDate = $nowtime - (60*$lastMinutes);
		$endDate = $nowtime;
        	$tmpList = $sensors->get_sensordata_range($data_sensor_name, $startDate, $endDate);
		$numX = count($tmpList)/10;
		$i=0;
		foreach ($tmpList as $row)
	        {
			if($i==0) $startTime = $row['datetime'];
	                $endTime = $row['datetime'];
			$sensorName = $row['name'];
			if($i%$numX==0) {
				$dataSet->addPoint(new Point(date("H:i", $row['datetime']), $row['txtdata']));
			}else{
				$dataSet->addPoint(new Point("", $row['txtdata']));
			}
	        	$i++;
		}
	}        
	$dateTitle = date("Y-m-d H:i", $startTime) . "~" . date("Y-m-d H:i", $endTime);
	
        $chart->setDataSet($dataSet);

        $chart->setTitle($dateTitle);
        $chart->render("generated/demo5.png");
?>
<!DOCTYPE HTML>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>phpChart - Basic Chart</title>
</head>
<body>
	<h3><?php echo $sensorName; ?></h3>
        <img alt="Line chart" src="generated/demo5.png" style="border: 1px solid gray;"/>
</body>
</html>

