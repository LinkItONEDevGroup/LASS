<?php

require "../conn/pdo.php"; 
require "../conn/conninfo.php"; 
require "functions.php";

$sensors   = new conninfo($pdo);

$lastMinutes = trim($_GET["min"]);
$typeSensor = trim($_GET["type"]); 

$endDate = time(); 
$startDate = $endDate - (60*$lastMinutes); 
$arrayDATA = ''; 
$arrayLABLE = '';
$arrayValueDisplay = '';
$arrayUNIT = ''; 
$tmpList = $sensors->get_sensordata_range($typeSensor, $startDate, $endDate); 

$counts = count($tmpList); 

$i=0; 

if($counts>0) {
	foreach ($tmpList as $row)
	{
		$i++;

		if($counts<6) {
			$arrayLABLE = $arrayLABLE . '"' . unixtime2Time($row['datetime']) . '"';
			$arrayValueDisplay = $arrayValueDisplay . '"' . $row['txtdata'] . $row['unit'] . '"';

		}else if($counts>5 and $counts<21) {
			if($i%2==1) {
                	        $arrayLABLE = $arrayLABLE . '"' . unixtime2Time($row['datetime']) . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"' . $row['txtdata'] . $row['unit'] . '"';
	                }else{
	                        $arrayLABLE = $arrayLABLE . '"\r\n' . unixtime2Time($row['datetime']) . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"\r\n' . $row['txtdata'] . $row['unit'] . '"';
	                }
		}else if($counts>20) {
			$numDISPLAY = ceil($counts/4);
			if($i%$numDISPLAY==1) {
				$arrayLABLE = $arrayLABLE . '"' . unixtime2Date($row['datetime']) . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"' . $row['txtdata'] . $row['unit'] . '"';
			}else{
				$arrayLABLE = $arrayLABLE . '""';
				$arrayValueDisplay = $arrayValueDisplay . '""';
			}
		}

		$arrayDATA = $arrayDATA . $row['txtdata'];
		$arrayUNIT = $row['unit'];
		
		if($i<$counts) {
			$arrayDATA = $arrayDATA . ', ';
			$arrayLABLE = $arrayLABLE . ', ';
			$arrayValueDisplay = $arrayValueDisplay . ', ';
		}
		
	}

	echo '{ "minv":' . $row['minv'] . ', "maxv":' . $row['maxv'] . ', "":"", "sensortype":"' . $row['typename'] . '", "title":"' . $row['moredesc'] . '", "unit": "' . $arrayUNIT . '", "labels": [' . $arrayLABLE . '], "data": [' . $arrayDATA . '], "display": [' . $arrayValueDisplay . '] }';
}	

?>
