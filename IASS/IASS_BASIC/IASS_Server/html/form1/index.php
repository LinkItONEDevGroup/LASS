
	<?php

    $jsonSensors = '
            { "sensors":[
              { "sensorIndex": 0,
				"sensorID": "MR",                         
                "sensorList": [ 1, 2, 3, 4] },                              
              { "sensorIndex": 1,
				"sensorID": "OR",                         
                "sensorList": [ 1, 2, 8, 10, 11] },
              { "sensorIndex": 2,
				"sensorID": "HR",                         
                "sensorList": [ 1, 2, 5, 6] }
            ]}                                           
            ';

    $SensorsList = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 ];
    

    require "../conn/pdo.php";
    require "../conn/conninfo.php";
	require "functions.php";
	$sensors   = new conninfo($pdo);

	require '../PHPMailer-master/PHPMailerAutoload.php';

	$email_sendout = false;  //是否要寄出email?

	$email_minutes_period = 15 * 60;   //每隔幾分鐘寄一次alarm email

	$tmpList = $sensors->get_email_time();

	$alarm_trigger = false;  //若有超過警戒值,則為true, 並執行mp3
	
	foreach ($tmpList as $row) {
		${ "email_last_time_".$row['idname'] } = $row['email_time'];
	}

        $A_datetime = $row['datetime'];

	$date = new DateTime();
	$nowTime = $date->getTimestamp();

    $output = shell_exec('./alarm.sh');
    echo "<pre>$output</pre>";
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html lang="tw"><head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"> 
<meta http-equiv="refresh" content="120" >
    <title>[凌陽創新] 環境偵測系統</title> 
    <style type="text/css">    
/*General styles*/
body
{
	margin: 0;
	padding: 0;
	background: white url(http://www.red-team-design.com/wp-content/themes/redv2/images/back.jpg) no-repeat left top;
	overflow-y: hidden;
	overflow-x: hidden;
}

a:hover, a:visited, a:link, a:active
{
    text-decoration: none;
}

#main
{
	width: 780px;
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
  height: 47px;
  line-height: 47px;
  padding: 0px 5px;
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

.features-table td:nth-child(0)
{
  /*background: #efefef;
  background: rgba(144,144,144,0.15); */
  border-right: 1px solid white;
}


.features-table td:nth-child(4), .features-table td:nth-child(3)
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
</head>
<body>

    <div id="main">
		<img src="header.png">
		<table class="features-table">
				<thead>
					<tr>
						<td align='right'></td>
<?php
$deviceSensorList = json_decode($jsonSensors, TRUE);

//Print table header columns
foreach($deviceSensorList['sensors'] as $key=>$val){
	
	$tmpList = $sensors->get_sensor_area($val['sensorID']);
    foreach ($tmpList as $row) { $typeName = $row['moredesc']; }
	
	$tmpList = $sensors->get_sensorupdate_time($val['sensorID']);
    foreach ($tmpList as $row) { $dateUpdate = unixtime2Date($row['datetime']); }
	
	$stringSensors = implode(":",$val['sensorList']);
    echo '<td><b>' . $typeName . '</b><br><font size="1" color="gray">' . $dateUpdate . '</td>';
}
echo '</tr></thead>';	

//select sensors which sensorID are necessary to display, and put them to $sensoridDisplay array.
$displaySensorList = [];
foreach ($SensorsList as $sensorType) {		
	$sensoridDisplay = [];
    foreach($deviceSensorList['sensors'] as $key=>$val){ 
		if(in_array($sensorType, $val['sensorList'])) $sensoridDisplay[$val['sensorIndex']] = $val['sensorID'];
	}	
	$displaySensorList[$sensorType] = $sensoridDisplay;
}

/*
print_r($deviceSensorList[sensors]);
echo '<hr>';
print_r($deviceSensorList[sensors][0]);
echo '<hr>';
print_r($displaySensorList);
*/

foreach ($SensorsList as $sensorType) {	
	$tmpList = $sensors->get_sensor_type($sensorType);
    foreach ($tmpList as $row)
    {
		$typeName = $row['name'];
		$typeUnit = $row['unit'];
	}
				
	$rawDisplay = '<tr><th>' . $typeName . '</th>';
	
	if(count($displaySensorList[$sensorType])>0) {
		for($iColumn=0;$iColumn<count($deviceSensorList[sensors]);$iColumn++) {
			if(empty($displaySensorList[$sensorType][$iColumn])) {
				$rawDisplay = $rawDisplay . '<td></td>';
			}else{
				$sensorID = $displaySensorList[$sensorType][$iColumn] . $sensorType;
				
				$tmpList = $sensors->get_sensor_list($sensorID);
				foreach ($tmpList as $row) { $alarm_MAX = $row['alarm_max']; $alarm_MIN = $row['alarm_min']; }
			
				$tmpList = $sensors->get_sensordata_now($sensorID);
				foreach ($tmpList as $row)
				{
					$datetimeSensor = $row['datetime'];
					$sensorValue = (int)$row['txtdata'];
					$statusIcon = ($sensorValue<=$alarm_MAX and $sensorValue>=$alarm_MIN)?"green.png":"red.png";

					if($sensorValue>$alarm_MAX) $alarm_trigger = true;
					$email_lasttime = ${ "email_last_time_".$sensorID };
					
				}

				$fmtDate = date("Y-m-d H:i", $datetimeSensor);
				
				$rawDisplay = $rawDisplay . '<td>' . '<a href="drawChart.php?min=60&type='.$sensorID.'"><img src="'.$statusIcon.'" width="20" height="20" alt="check"><br>'.'<font color="blue"><b>'.$sensorValue.'</b></font><font color="black">'.$typeUnit.'</font></a>';
				
				
			}
				
		}

		echo $rawDisplay;
	}
	
	echo '</tr>';
}
?>
		</table>
		<img src="footer.png">
	</div>
 </body>
</html>
<?php
	$conn = null;
	if($alarm_trigger == true) {
		echo "<embed src=\"http://localhost/sensors/form1/alarm.mp3\" autostart=\"true\" loop=\"true\" hidden=\"true\"></embed>\n"
	    ."<noembed><bgsound src=\"alarm.mp3\" loop=\"infinite\"></noembed>";
	}

?>
</body>

</html>
