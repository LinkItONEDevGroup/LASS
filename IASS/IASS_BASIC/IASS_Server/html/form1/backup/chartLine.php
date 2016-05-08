<?php
        require "../conn/pdo.php";
        require "../conn/conninfo.php";
        require "functions.php";

        $sensors   = new conninfo($pdo);
        //$data_sensor_name = trim($_GET["sensor"]);
        $lastMinutes = trim($_GET["min"]);
        $typeSensor = trim($_GET["type"]);
        $numdata = trim($_GET["min"]);

        switch ($typeSensor) {
                //Display only one line for a sensor
                case "a1":
                        $arraySensors = array("A1");
                        $lineTitle = "溫度曲線圖";
                        $y_title = "溫度(°C)";
                        $txtUnit = "°C";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "a2":
                        $arraySensors = array("A2");
                        $lineTitle = "溼度曲線圖";
                        $y_title = "溼度(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "a3":
                        $arraySensors = array("A3");
                        $lineTitle = "漏水機率曲線圖";
                        $y_title = "漏水機率(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "a4":
                        $arraySensors = array("A4");
                        $lineTitle = "噪音音量曲線圖";
                        $y_title = "噪音(dB)";
                        $txtUnit = "dB";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "a5":
                        $arraySensors = array("A5");
                        $lineTitle = "煙霧曲線圖";
                        $y_title = "煙霧(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b1":
                        $arraySensors = array("B1");
                        $lineTitle = "溫度曲線圖";
                        $y_title = "溫度(°C)";
                        $txtUnit = "°C";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b2":
                        $arraySensors = array("B2");
                        $lineTitle = "溼度曲線圖";
                        $y_title = "溼度(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b3":
                        $arraySensors = array("B3");
                        $lineTitle = "漏水機率曲線圖";
                        $y_title = "漏水機率(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b4":
                        $arraySensors = array("B4");
                        $lineTitle = "噪音音量曲線圖";
                        $y_title = "噪音(dB)";
                        $txtUnit = "dB";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b5":
                        $arraySensors = array("B5");
                        $lineTitle = "煙霧曲線圖";
                        $y_title = "煙霧(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "b7":
                        $arraySensors = array("B7", "O7", "S7");
                        $lineTitle = "PM2.5曲線圖";
                        $y_title = "PM2.5(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                case "b8":
                        $arraySensors = array("B8", "S8");
                        $lineTitle = "CO2曲線圖";
                        $y_title = "CO2(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0, trace1];";
                        break;
                case "b9":
                        $arraySensors = array("B9","O9");
                        $lineTitle = "PM10曲線圖";
                        $y_title = "PM10(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1];";
                        break;
                case "c1":
                        $arraySensors = array("C1");
                        $lineTitle = "溫度曲線圖";
                        $y_title = "溫度 (°C)";
                        $txtUnit = "°C";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c2":
                        $arraySensors = array("C2");
                        $lineTitle = "溼度曲線圖";
                        $y_title = "溼度(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c3":
                        $arraySensors = array("C3");
                        $lineTitle = "漏水機率曲線圖";
                        $y_title = "漏水機率(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c4":
                        $arraySensors = array("C4");
                        $lineTitle = "噪音音量曲線圖";
                        $y_title = "噪音(dB)";
                        $txtUnit = "dB";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c5":
                        $arraySensors = array("C5");
                        $lineTitle = "煙霧曲線圖";
                        $y_title = "煙霧(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c5":
                        $arraySensors = array("C5");
                        $lineTitle = "煙霧曲線圖";
                        $y_title = "煙霧(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "c7":
                        $arraySensors = array("C7","O7", "S7");
                        $lineTitle = "PM2.5曲線圖";
                        $y_title = "PM2.5(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                case "c8":
                        $arraySensors = array("C8", "S8");
                        $lineTitle = "CO2曲線圖";
                        $y_title = "CO2(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0, trace1];";
                        break;
                case "c9":
                        $arraySensors = array("C9","O9");
                        $lineTitle = "PM10曲線圖";
                        $y_title = "PM10(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1];";
                        break;
                //Display all data line for every sensor
                case "1":
                        $arraySensors = array("A1","B1","C1");
                        $lineTitle = "各偵測點溫度曲線圖";
                        $y_title = "溫度(°C)";
                        $txtUnit = "°C";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                case "2":
                        $arraySensors = array("A2","B2","C2");
                        $lineTitle = "各偵測點溼度曲線圖";
                        $y_title = "溼度(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                case "3":
                        $arraySensors = array("A3");
                        $lineTitle = "漏水機率曲線圖";
                        $y_title = "漏水機率(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "4":
                        $arraySensors = array("B4","C4");
                        $lineTitle = "各偵測點噪音曲線圖";
                        $y_title = "噪音(dB)";
                        $txtUnit = "dB";
			$txtPlotJavascript = "var data = [trace0, trace1];";
                        break;
                case "5":
                        $arraySensors = array("A5");
                        $lineTitle = "各偵測點煙霧曲線圖";
                        $y_title = "煙霧(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0];";
                        break;
                case "7":
                        $arraySensors = array("B7","C7","O7","S7");
                        $lineTitle = "各偵測點PM2.5曲線圖";
                        $y_title = "PM2.5(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2, trace3];";
                        break;
                case "8":
                        $arraySensors = array("B8","C8", "S8");
                        $lineTitle = "各偵測點CO2曲線圖";
                        $y_title = "CO2(ppm)";
                        $txtUnit = "ppm";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                case "9":
                        $arraySensors = array("B9","C9","O9");
                        $lineTitle = "各偵測點PM10曲線圖";
                        $y_title = "PM10(μg/m3)";
                        $txtUnit = "μg/m3";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
                        break;
                default:
                        $arraySensors = array("A2","B2","C2");
                        $lineTitle = "各偵測點溼度曲線圖";
                        $y_title = "溼度(%)";
                        $txtUnit = "%";
			$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
        }
        $arrayNum = 0;
        $sensorName = array();
        $jsonCategories = array();
        $jsonData = array();
        //if(strlen($data_sensor_name)>0) {
        foreach ($arraySensors as $data_sensor_name) {
                if(strlen($lastMinutes)>0) {
                        $nowtime = time();
                        $startDate = $nowtime - (60*$lastMinutes);
                        $endDate = $nowtime;
                        $tmpList = $sensors->get_sensordata_range($data_sensor_name, $startDate, $endDate);
                }else{
                        $tmpList = $sensors->get_sensordata_num($data_sensor_name, $numdata);
                }
		if(count($tmpList )>0) {
	                $i=0;
	                $jsonCategories[$arrayNum] = '[';
	                $jsonData[$arrayNum] = '[';
	                foreach ($tmpList as $row)
	                {
	                        //將漏水值改為機率
	                        if($typeSensor=="a3" OR $typeSensor=="3") {
	                                $valuePercent = waterPercent((int)$row['txtdata']);
	                        }else{
	                                $valuePercent =  $row['txtdata'];
	                        }
	                        if($i==0) $startTime = $row['datetime'];
	                        $endTime = $row['datetime'];
	                        $sensorName[$arrayNum] = $row['name'];
	                        $txtSensorName = $row['name'];
	                        if($i>0) {
	                                $jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . ', ';
        	                        $jsonData[$arrayNum] = $jsonData[$arrayNum] .  ', ';
	                        }
                	        $jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "'".date("Y-m-d H:i:00", $row['datetime'])."'";
	                        $jsonData[$arrayNum] = $jsonData[$arrayNum] . $valuePercent;
	                        $i++;
        	        }
                	$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "]";
        	        $jsonData[$arrayNum] = $jsonData[$arrayNum] .  "]";
			//echo $jsonData[$arrayNum] . "<br>";
                	$arrayNum++;
			if($arrayNum==1) {
				$txtPlotJavascript = "var data = [trace0];";
			}else if($arrayNum==2) {
				$txtPlotJavascript = "var data = [trace0, trace1];";
			}else if($arrayNum==3) {
				$txtPlotJavascript = "var data = [trace0, trace1, trace2];";
			}else if($arrayNum==4) {
                                $txtPlotJavascript = "var data = [trace0, trace1, trace2, trace3];";
                        }
		}
	}
        $dateTitle = date("Y-m-d H:i", $startTime) . "~" . date("Y-m-d H:i", $endTime);
        $lineTitle = "SunplusIT " . $lineTitle;
        //if(strlen($typeSensor)>1) $lineTitle=$dateTitle . " " . $txtSensorName;
	
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html lang="tw"><head>
<head>
  <meta http-equiv="content-type" content="text/html; charset=UTF-8">
  <meta http-equiv="refresh" content="120" >
  <meta name="robots" content="noindex">
  <title><?php echo "凌陽創新環控系統-".$dateTitle; ?></title>
<head>
  <!-- Plotly.js -->
  <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
</head>
<?php
        $numdata_s = $numdata - 60; if($numdata_s<61) $numdata_s=60;
        $numdata_l = $numdata + 60; if($numdata_l>10080) $numdata_l=10080;
?>

<body>
<center> <div id="main">
                <a href="index.php"><img src="header1.png" width="670" height="45" border="0"></a>
                <?php if($numdata>60) { ?><a href="chartLine.php?min=<?php echo $numdata_s;?>&type=<?php echo $typeSensor; ?>">
                                <img src="smaller.png" width="45" height="45" border="0"></a><?php } ?>
                                <?php if($numdata<10080) { ?><a href="chartLine.php?min=<?php echo $numdata_l; ?>&type=<?php echo $typeSensor; ?>">
                                <img src="larger.png" width="45" height="45" border="0"></a><?php } ?>
                <table class="features-table">
                        <tbody><tr><td>
			<div id="myDiv" style="width: 780px; height: 400px;"><!-- Plotly chart will be drawn inside this DIV --></div>
			  <script>
			    <!-- JAVASCRIPT CODE GOES HERE -->
<?php

        $i = 0;
        foreach ($sensorName as $sensorArea) {
?>
        var trace<?php echo $i; ?> = {
  x: <?php echo $jsonCategories[$i]; ?>,
  y: <?php echo $jsonData[$i]; ?>,
  type: 'scatter',
  name: '<?php echo $sensorArea; ?>'
};
<?php
        $i++;
}

	echo $txtPlotJavascript;
?>

			Plotly.newPlot('myDiv', data);
			  </script>
                        </td></tr></tbody>
                </table>

                </p><img src="footer.png">
</div></center>
</body>

</html>



