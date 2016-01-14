<?php
// Configuration
$dbhost = 'lass.iis.sinica.edu.tw';
$dbname = 'LASS';
$device_id = $_GET['device_id'];
 
// Connect to mongo database
$mongoClient = new MongoClient('mongodb://' . $dbhost);
$db = $mongoClient->$dbname;
$collection = $db->posts;



$sites = $collection->distinct('SiteName', array('date'=> date("Y-m-d")));

$msgs = array();
foreach ($sites as $site){

	$items = $collection->find(array('SiteName' => $site, 
				         'date'=>array('$lte'=> date("Y-m-d")))
				  )->sort(array('date'=> -1, 'time'=>-1))->limit(1);

	$msg = array('SiteName' => $site);

	foreach ($items as $item){
		$timestamp = $item['date']."T".$item['time']."Z";
		$msg['timestamp'] = $timestamp;
		if (isset($item['PM2.5'])) $msg['PM2.5'] = $item['PM2.5'];
		if (isset($item['PM10'])) $msg['PM10'] = $item['PM10'];
		if (isset($item['SO2'])) $msg['SO2'] = $item['SO2'];
		if (isset($item['NOx'])) $msg['NOx'] = $item['NOx'];
		if (isset($item['NO2'])) $msg['NO2'] = $item['NO2'];
		if (isset($item['NO'])) $msg['NO'] = $item['NO'];
		if (isset($item['PSI'])) $msg['PSI'] = $item['PSI'];
		$location = $item['loc']['coordinates'];
		$msg['gps_lat'] = $location[0];
		$msg['gps_lon'] = $location[1];
	}

	array_push($msgs, $msg);

}

$msg = array('source' => "last-all-epa", 'feeds' => $msgs);

echo json_encode($msg);

?>
