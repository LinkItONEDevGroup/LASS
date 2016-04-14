<?php
    require "../conn/pdo.php";
    require "../conn/conninfo.php";
	require "functions.php";

    $sensors   = new conninfo($pdo);
    //$data_sensor_name = trim($_GET["sensor"]);
    $lastMinutes = trim($_GET["min"]);
	$typeSensor = trim($_GET["type"]);
	$numdata = trim($_GET["num"]);
	
	$tmpList = $sensors->get_sensor_list($typeSensor);
    foreach ($tmpList as $row)
    {
		$typeName = $row['name'];
		$typeUnit = $row['unit'];
	}
	
	$arraySensors = array("\"" . $typeSensor . "\"");
	$lineTitle = $typeName . "曲線圖";
	$y_title = $typeUnit;
	$txtUnit = $typeUnit;

	$arrayNum = 0;
	$sensorName = array();
	$jsonCategories = array();
	$jsonData = array();
        //if(strlen($data_sensor_name)>0) {
	print_r($arraySensors);
	foreach ($arraySensors as $data_sensor_name) {
		if(strlen($lastMinutes)>0) {
			$nowtime = time();
			$startDate = $nowtime - (60*$lastMinutes);
			$endDate = $nowtime;
			$tmpList = $sensors->get_sensordata_range($data_sensor_name, $startDate, $endDate);
		}else{
			$tmpList = $sensors->get_sensordata_num($data_sensor_name, $numdata);
		}
		//echo 'TEST:'. $data_sensor_name . '/' . $startDate . '/' . $endDate;
        $i=0;
		$jsonCategories[$arrayNum] = '[';
		$jsonData[$arrayNum] = '[';
        foreach ($tmpList as $row)
        {

			if($i==0) $startTime = $row['datetime'];
			$endTime = $row['datetime'];
			$sensorName[$arrayNum] = $row['name'];
			$txtSensorName = $row['name'];
			if($i>0) {
				$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . ', ';
				$jsonData[$arrayNum] = $jsonData[$arrayNum] .  ', ';
			}
			$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "'".date("H:i", $row['datetime'])."'";
			$jsonData[$arrayNum] = $jsonData[$arrayNum] . $valuePercent;
			$i++;
		}
		$jsonCategories[$arrayNum] = $jsonCategories[$arrayNum] . "]";
		$jsonData[$arrayNum] = $jsonData[$arrayNum] .  "]";
		$arrayNum++;
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
  <script type='text/javascript' src='../hicharts/jquery-2.1.4.min.js'></script>
    <style type="text/css">
/*General styles*/
body
{
        margin: 0;
        padding: 0;
        background: white url(http://www.red-team-design.com/wp-content/themes/redv2/images/back.jpg) no-repeat left top;
}

#main
{
        width: 770px;
        margin: 0 auto 0 auto;
        background: white;
        -moz-border-radius: 8px;
        -webkit-border-radius: 8px;
        padding: 10px;
        border: 1px solid #adaa9f;
        -moz-box-shadow: 0 2px 2px #9c9c9c;
        -webkit-box-shadow: 0 2px 2px #9c9c9c;
}

/*Features table------------------------------------------------------------*/
.features-table
{
  width: 100%;
  margin: 0 auto;
  border-collapse: separate;
  border-spacing: 0;
  text-shadow: 0 1px 0 #fff;
  color: #2a2a2a;
  background: #fafafa;
  background-image: -moz-linear-gradient(top, #fff, #eaeaea, #fff); /* Firefox 3.6 */
  background-image: -webkit-gradient(linear,center bottom,center top,from(#fff),color-stop(0.5, #eaeaea),to(#fff));
}

.features-table td
{
  height: 50px;
  line-height: 50px;
  padding: 0 20px;
  border-bottom: 1px solid #cdcdcd;
  box-shadow: 0 1px 0 white;
  -moz-box-shadow: 0 1px 0 white;
  -webkit-box-shadow: 0 1px 0 white;
  white-space: nowrap;
  text-align: center;
}

/*Body*/
.features-table tbody td
{
  text-align: center;
  font: normal 12px Verdana, Arial, Helvetica;
  width: 150px;
}

.features-table tbody td:first-child
{
  width: auto;
  text-align: left;
}

.features-table td:nth-child(2), .features-table td:nth-child(3)
{
  background: #efefef;
  background: rgba(144,144,144,0.15);
  border-right: 1px solid white;
}
.features-table td:nth-child(4)
{
  background: #e7f3d4;
  background: rgba(184,243,85,0.3);
}

/*Header*/
.features-table thead td
{
  font: bold 1.3em 'trebuchet MS', 'Lucida Sans', Arial;
  -moz-border-radius-topright: 10px;
  -moz-border-radius-topleft: 10px;
  border-top-right-radius: 10px;
  border-top-left-radius: 10px;
  border-top: 1px solid #eaeaea;
}

.features-table thead td:first-child
{
  border-top: none;
}

/*Footer*/
.features-table tfoot td
{
  font: bold 1.4em Georgia;
  -moz-border-radius-bottomright: 10px;
  -moz-border-radius-bottomleft: 10px;
  border-bottom-right-radius: 10px;
  border-bottom-left-radius: 10px;
  border-bottom: 1px solid #dadada;
}

.features-table tfoot td:first-child
{
  border-bottom: none;
}
    </style>


<script type='text/javascript'>//<![CDATA[

$(function () {

    $('#container').highcharts({
        title: {
            text: '<?php echo $dateTitle; ?>',
            x: -20 //center
        },
        subtitle: {
            text: '<?php echo $lineTitle; ?>',
            x: -20
        },
        xAxis: {
            categories: <?php echo $jsonCategories[0]; ?>
        },
        yAxis: {
            title: {
                text: '<?php echo $y_title; ?>'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            valueSuffix: '<?php echo $txtUnit; ?>'
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'middle',
            borderWidth: 0
        },
        series: [
	<?php 
		$i = 0;
		foreach ($sensorName as $sensorArea) {
			if($i>0) echo ", ";
	?>
	{
            name: '<?php echo $sensorArea; ?>',
            data: <?php echo $jsonData[$i]; 
	    $i++;	
	?>
        }
	   <?php } ?>
        ]
    });
});
//]]> 

</script>

</head>
<?php
	$numdata_s = $numdata - 600; if($numdata_s<601) $numdata_s=600; 
	$numdata_l = $numdata + 600; if($numdata_l>4320) $numdata_l=4320;
?>
<body>
    <div id="main">
                <a href="index.php"><img src="header1.png" width="670" height="45" border="0"></a>
		<?php if($numdata>600) { ?><a href="chart_line.php?num=<?php echo $numdata_s;?>&type=<?php echo $typeSensor; ?>">
                                <img src="smaller.png" width="45" height="45" border="0"></a><?php } ?>
                                <?php if($numdata<4320) { ?><a href="chart_line.php?num=<?php echo $numdata_l; ?>&type=<?php echo $typeSensor; ?>">
                                <img src="larger.png" width="45" height="45" border="0"></a><?php } ?>
                <table class="features-table">
			<tbody><tr><td>
			<?php //echo $jsonCategories[0]; ?>
			<?php //echo "<br>".$jsonData[0]; ?>
			<script src="../hicharts/js/highcharts.js"></script>
			<script src="../hicharts/modules/exporting.js"></script>

			<div id="container" style="min-width: 310px; height: 400px; margin: 0 auto"></div>
			</td></tr></tbody>
		</table>
 
		</p><img src="footer.png">
</body>

</html>
