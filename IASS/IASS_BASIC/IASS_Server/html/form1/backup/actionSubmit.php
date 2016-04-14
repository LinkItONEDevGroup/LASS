<?php
        require "../conn/pdo.php";
        require "../conn/conninfo.php";
        require "functions.php";
        $sensors   = new conninfo($pdo);

	$maxA1 = $_POST['maxA1'];
	$maxA2 = $_POST['maxA2'];
	$maxA5 = $_POST['maxA5'];
	$maxB1 = $_POST['maxB1'];
	$maxB2 = $_POST['maxB2'];
	$maxB7 = $_POST['maxB7'];
	$maxB8 = $_POST['maxB8'];
	$maxC1 = $_POST['maxC1'];
        $maxC2 = $_POST['maxC2'];
        $maxC7 = $_POST['maxC7'];
        $maxC8 = $_POST['maxC8'];

	$tmpList = $sensors->update_alarm_config("A1", $maxA1);
	$tmpList = $sensors->update_alarm_config("A2", $maxA2);
	$tmpList = $sensors->update_alarm_config("A5", $maxA5);
        $tmpList = $sensors->update_alarm_config("B1", $maxB1);
	$tmpList = $sensors->update_alarm_config("B2", $maxB2);
        $tmpList = $sensors->update_alarm_config("B7", $maxB7);
	$tmpList = $sensors->update_alarm_config("B8", $maxB8);
	$tmpList = $sensors->update_alarm_config("C1", $maxC1);
        $tmpList = $sensors->update_alarm_config("C2", $maxC2);
        $tmpList = $sensors->update_alarm_config("C7", $maxC7);
        $tmpList = $sensors->update_alarm_config("C8", $maxC8);
	$conn = null;

	header('Location: alarmconfig.php?update=OK');   
?>
