<?php
$servername = "localhost";
$username = "pi";
$password = "1qaz";
$dbname = "sensor_data";

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
              "sensorList": [ 1, 2, 5, 6] 
			}
        ]
	}';

    $SensorsList = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 ];


global $conn;

try {
    $pdo = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    $pdo->exec("set names utf8");
    // set the PDO error mode to exception
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    //echo "Connected successfully"; 
    }

catch(PDOException $e)
    {
    echo "Connection failed: " . $e->getMessage();
    }
