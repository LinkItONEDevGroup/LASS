<?php
// Configuration
$dbhost = 'your_mongodb_server';
$dbname = 'LASS';
$device_id = $_GET['device_id'];
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;

$items = $collection->find(array('device_id' => $device_id, 
			         'date'=>array('$lte'=> date("Y-m-d")))
			  )->sort(array('date'=> -1, 'time'=>-1))->limit(1);

$msg = array('device_id' => $device_id);

foreach ($items as $item){
	$timestamp = $item['date']."T".$item['time']."Z";
	$msg['timestamp'] = $timestamp;
	$msg['s_d0'] = $item['s_d0'];
	$msg['s_t0'] = $item['s_t0'];
	$msg['s_h0'] = $item['s_h0'];
}

$items = $collection->find(array('device_id' => $device_id, 
				 'gps_fix' => 1,
			         'date'=>array('$lte'=> date("Y-m-d")))
			  )->sort(array('gps_num' => -1, 'date'=> -1, 'time'=>-1))->limit(1);
foreach ($items as $item){
	$location = $item['loc']['coordinates'];
	$msg['gps_lat'] = $location[0];
	$msg['gps_lon'] = $location[1];
	$msg['gps_num'] = $item['gps_num'];
}

echo json_encode($msg);

?>
