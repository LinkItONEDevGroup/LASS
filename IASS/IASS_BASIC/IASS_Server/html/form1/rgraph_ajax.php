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

		if($counts<=4) {
			$arrayLABLE = $arrayLABLE . '"' . $row['datetime'] . '"';
			$arrayValueDisplay = $arrayValueDisplay . '"' . round($row['txtdata'],1) . $row['unit'] . '"';

		}else if($counts>4 and $counts<=10) {
			if($i%2==0) {
                	        $arrayLABLE = $arrayLABLE . '"' . $row['datetime'] . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"' . round($row['txtdata'],1) . $row['unit'] . '"';
	                }else{
	                        $arrayLABLE = $arrayLABLE . '"\r\n' . $row['datetime'] . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"\r\n' . round($row['txtdata'],1) . $row['unit'] . '"';
	                }
		}else if($counts>10) {
			$numDISPLAY = ceil($counts/4);
			if($i%$numDISPLAY==0 or $counts==$i) {
				$arrayLABLE = $arrayLABLE . '"' . $row['datetime'] . '"';
				$arrayValueDisplay = $arrayValueDisplay . '"' . round($row['txtdata'],1) . $row['unit'] . '"';
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
