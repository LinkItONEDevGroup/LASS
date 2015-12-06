<?php
// Configuration
$dbhost = 'your mongodb server';
$dbname = 'LASS';
$device_id = $_GET['device_id'];
$device_date = $_GET['date'];
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;

$items = $collection->find(array('device_id' => $device_id, 
			         'date'=> $device_date)
			  )->sort(array('date'=> -1, 'time'=>-1));

$feeds = array();

foreach ($items as $item){
	array_push($feeds, array('time' => $item['time'], 's_d0' => $item['s_d0'], 's_d1' => $item['s_d1']));
}

$msg = array('device_id' => $device_id, 'feeds' => $feeds);
echo json_encode($msg);

?>
