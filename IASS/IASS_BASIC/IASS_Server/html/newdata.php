<?php
    require "conn/pdo.php";
	require "conn/conninfo.php";

        function sensorID_Check($sensorNameID) {
		$sensor_id = $sensors->get_sensor_list($sensorNameID);
		foreach ($sensor_id as $row)
		{
		    echo $row['id'];
		}
	/*
                $sql = $conn->prepare("select id from sensor_list where idname='$sensorNameID'");
                $sql->execute();

                $result = $sql->setFetchMode(PDO::FETCH_ASSOC);
                foreach(new TableRows(new RecursiveArrayIterator($sql->fetchAll())) as $k=>$v) {
       	                echo "Found sensor nameID is ".$v;
                }

                if($v>0) {
                        return $v;
                }else{
                        return 0;
                }
	*/
        }

	$sensors   = new conninfo($pdo);
    	$data_sensor_name = trim($_GET["sensor"]) . trim($_GET["type"]);
	$data_device_name = trim($_GET["sensor"]);
	$data_desc = trim($_GET["device"]);
	$data_sensor_type = trim($_GET["type"]);
    	$data_sensor_value = trim($_GET["vdata"]);
	$sensorID = 0;

        if(strlen($data_sensor_name)>0 AND strlen($data_sensor_value)>0) {
		//Get this sensor's ID
		$tmpList = $sensors->get_sensor_list($data_sensor_name);
                foreach ($tmpList as $row)
                {
                    $sensorID = $row['id'];
                }

		if(!$sensorID>0) {
			$nowtime = time();
			$sql = "INSERT INTO sensor_list (idname, name, moredesc, type_id, create_date)
	                                VALUES ('$data_sensor_name', '$data_device_name', '$data_desc', '$data_sensor_type', '$nowtime')";
	                        // use exec() because no results are returned
	                        echo $sql;
	                        $pdo->exec($sql);
	                        echo "New sensor ID created successfully!";

		}
		
                $nowtime = time();
		
	        $sql = "INSERT INTO data_storage (sensor_id, datetime, txtdata)
        	        VALUES ('$sensorID', '$nowtime', '$data_sensor_value')";
                // use exec() because no results are returned
	        echo $sql;
	        $pdo->exec($sql);
	        echo "New data from sensor created successfully!";
		
        }

        $conn = null;
?>

