<?php

require "../conn/pdo.php"; 
require "../conn/conninfo.php"; 
require "functions.php";

$sensors   = new conninfo($pdo);

$lastMinutes = trim($_GET["min"]);
$typeSensor = trim($_GET["type"]); 

$endDate = time(); 
$startDate = $endDate - (60*$lastMinutes); 
$arrayDATA = []; 
$arrayLABLE = []; 
$tmpList = $sensors->get_sensordata_range($typeSensor, $startDate, $endDate); 

$counts = count($tmpList); 

$i=0; 

if($counts>0) {
	foreach ($tmpList as $row)
	{
		$i++;
		//$arrayLABLE = $arrayLABLE . "'" . unixtime2Date($row['datetime']) . "'";
		//$arrayDATA = $arrayDATA . $row['txtdata'];
		//$unit = $row['unit'];

		array_push($arrayDATA, (int)$row['txtdata']);
		array_push($arrayLABLE, "'".unixtime2Date($row['datetime'])."'");
		/*
		if($i<$counts) {
			$arrayDATA = $arrayDATA . ', ';
			$arrayLABLE = $arrayLABLE . ', ';
		}
		*/
	}

	echo '{ "labels": "[' . implode(", ",$arrayLABLE) . ']", "data": "[' . implode(", ",$arrayDATA) . ']" }';
}	

?>
