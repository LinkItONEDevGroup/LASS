<?php
// Configuration
$dbhost = 'your mongodb server';
$dbname = 'LASS';
$device_id = $_GET['device_id'];
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;

$items = $collection->find(array('device_id' => $device_id, 
			         'date'=>array('$lte'=> date("Y-m-d")))
			  )->sort(array('date'=> -1, 'time'=>-1))->limit(1000);

$feeds = array();

foreach ($items as $item){
	$timestamp = $item['date']."T".$item['time']."Z";
	array_push($feeds, array('timestamp' => $timestamp, 's_d0' => $item['s_d0']));
}

$msg = array('device_id' => $device_id, 'feeds' => $feeds);
echo json_encode($msg);

?>
