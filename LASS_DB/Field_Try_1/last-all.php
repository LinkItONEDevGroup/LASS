<?php
// Configuration
$dbhost = 'lass.iis.sinica.edu.tw';
$dbname = 'LASS';
$device_id = $_GET['device_id'];
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;



$devices = $collection->distinct('device_id', array('date'=> date("Y-m-d")));

$msgs = array();
foreach ($devices as $device_id){

	$items = $collection->find(array('device_id' => $device_id, 
				         'date'=>array('$lte'=> date("Y-m-d")))
				  )->sort(array('date'=> -1, 'time'=>-1))->limit(1);

	$msg = array('device_id' => $device_id);

	foreach ($items as $item){
		$timestamp = $item['date']."T".$item['time']."Z";
		$msg['timestamp'] = $timestamp;
		if (isset($item['s_d0'])) $msg['s_d0'] = $item['s_d0'];
		if (isset($item['s_d1'])) $msg['s_d1'] = $item['s_d1'];
		if (isset($item['s_t0'])) $msg['s_t0'] = $item['s_t0'];
		if (isset($item['s_h0'])) $msg['s_h0'] = $item['s_h0'];
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

	array_push($msgs, $msg);

}

$msg = array('source' => "last-all", 'feeds' => $msgs);

echo json_encode($msg);

?>
