<?php
// Configuration
$dbhost = '140.109.21.181';
$dbname = 'LASS';
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;

$date_today = date("Y-m-d");
$date_yesterday = date("Y-m-d",time() - 60 * 60 * 24);

$devices = $collection->distinct('device_id');
$msgs = array();

foreach ($devices as $device_id){

	$items = $collection->find(array('device_id' => $device_id, 
				'date' => array( '$in' => array($date_today, $date_yesterday))
//				'date' => $date_today
					)
				  )->sort(array('date'=> -1, 'time'=>-1))->limit(1);
/*
	$array_items = iterator_to_array($items);
	if (!isset($array_items[0]['date'])){
		$items = $collection->find(array('device_id' => $device_id, 
						'date' => $date_yesterday
						)
					  )->sort(array('date'=> -1, 'time'=>-1))->limit(1);
	$array_items = iterator_to_array($items);
	}
	//if (!isset($array_items[0]['date'])) continue;
*/
	if (count($items)==0) continue;
	$msg = array('device_id' => $device_id);
	foreach ($items as $item){
		$s_date = $item['date'];
		$s_time = $item['time'];
		$timestamp = $item['date']."T".$item['time']."Z";
		$msg['timestamp'] = $timestamp;
		if (isset($item['s_d0'])) $msg['s_d0'] = $item['s_d0'];
		if (isset($item['s_d1'])) $msg['s_d1'] = $item['s_d1'];
		if (isset($item['s_t0'])) $msg['s_t0'] = $item['s_t0'];
		if (isset($item['s_h0'])) $msg['s_h0'] = $item['s_h0'];
		if (isset($item['gps_fix'])){
			if ($item['gps_fix']==1 && isset($item['loc'])){
				$location = $item['loc']['coordinates'];
				$msg['gps_lat'] = $location[0];
				$msg['gps_lon'] = $location[1];
				$msg['gps_num'] = $item['gps_num'];
			}
		}
	}

	if (isset($msg['gps_num'])) array_push($msgs, $msg);

}

$msg = array('source' => "last-all", 'feeds' => $msgs);

echo json_encode($msg);

?>
