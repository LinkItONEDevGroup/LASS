<?php
        require "../conn/pdo.php";
        require "../conn/conninfo.php";

	require_once("../../phpChart_Lite/conf.php");

	$sensors   = new conninfo($pdo);
        $data_sensor_name = trim($_GET["sensor"]);
?>
<!DOCTYPE HTML>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>phpChart - Basic Chart</title>
</head>
  
<body>
<?php
if(strlen($data_sensor_name)>0) {
	$tmpList = $sensors->get_sensordata_range($data_sensor_name, 60);
	$arrayData = array();
	$i=0;
	foreach ($tmpList as $row)
        {
                $arrayData[$i] = $row['txtdata'];
		if($i==0) $startTime = $row['datetime'];
		$endTime = $row['datetime'];
		$i++;
        }
	$dateTitle = date("Y-m-d H:i", $startTime) . "~" . date("Y-m-d H:i", $endTime) . "溫度趨線圖";
	//print_r($arrayData);
	$pc = new C_PhpChartX(array($arrayData),'basic_chart');
	$pc->set_animate(true);
	$pc->set_title(array('text'=>$dateTitle));
	//$pc->set_axes(array('yaxis'=> array('min'=>0,'max'=>100,'padMax'=>5.0)));
//set axes
$pc->set_xaxes(array(
    'xaxis'  => array(
        'borderWidth'=>2,
        'borderColor'=>'#999999', 
        'tickOptions'=>array('showGridline'=>false))
    ));

$pc->set_yaxes(array(
    'yaxis' => array(
        'borderWidth'=>0,
        'borderColor'=>'#ffffff', 
        'autoscale'=>true, 
        'min'=>'0', 
        'max'=>100, 
        'numberTicks'=>21,
        'labelRenderer'=>'plugin::CanvasAxisLabelRenderer',
        'label'=>'Energy Use')
    ));
	$pc->set_series_default(array('linePattern'=>'dashed','showMarker'=>false,'shadow'=>false));
	$pc->draw();
}
?>

</body>
</html>

