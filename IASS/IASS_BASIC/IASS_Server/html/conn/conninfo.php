<?php

class conninfo{

    function __construct($db)
    {
        $this->db = $db;
    }
	
    function get_sensor_type($typeID)
    {
        $sql = "select * from sensor_type where id='$typeID'";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($typeID));
        return $stm->fetchAll();
    }	
	
	function get_sensor_area($sensorName)
    {
        $sql = "select * from sensor_list where name='$sensorName'";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($sensorName));
        return $stm->fetchAll();
    }

    function get_sensor_list($sensorNameID)
    {
        $sql = "select * from sensor_list where idname='$sensorNameID'";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($sensorNameID));
        return $stm->fetchAll();
    }

    function get_sensordata_list($sensorID)  //no use ....
    {
        $sql = "select a.datetime,a.txtdata,b.idname,b.name from data_storage a, sensor_list b where a.sensor_id='$sensorID' and a.sensor_id=b.id order by datetime desc limit 0,1";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($sensorID));
        return $stm->fetchAll();
    }
	
	function get_sensorupdate_time($deviceName)
    {
        $sql = "select a.datetime,a.txtdata,b.idname,b.name from data_storage a, sensor_list b where b.name='$deviceName' and a.sensor_id=b.id order by a.datetime desc limit 0,1";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($deviceName));
        return $stm->fetchAll();
    }

    function get_sensordata_now($sensorNameID)
    {
        $sql = "select a.datetime,a.txtdata,b.idname,b.name from data_storage a, sensor_list b where b.idname='$sensorNameID' and a.sensor_id=b.id order by a.datetime desc limit 0,1";
        $stm = $this->db->prepare($sql);
        $stm->execute(array($sensorNameID));
        return $stm->fetchAll();
    }

    function get_sensordata_num($sensorNameID, $num)
    {
        $sql = "SELECT * FROM ( SELECT a.datetime,a.txtdata,b.name FROM data_storage a,sensor_list b where b.idname='$sensorNameID' and  a.sensor_id=b.id ORDER BY a.datetime DESC LIMIT 0,$num ) sub ORDER BY datetime ASC";
        //echo $sql."<br>";
	$stm = $this->db->prepare($sql);
        $stm->execute();
        return $stm->fetchAll();
    }

    function get_sensordata_range2($sensorNameID, $startDate, $endDate)
    {
        $sql = "SELECT * FROM ( SELECT a.datetime,a.txtdata,b.name, b.moredesc, c.unit, c.name as typename, c.maxv, c.minv FROM data_storage a,sensor_list b, sensor_type c where (a.datetime<=$endDate and a.datetime>=$startDate) and (b.idname='$sensorNameID' and  a.sensor_id=b.id and c.id=b.type_id)) sub ORDER BY datetime ASC";
	//echo $sql."<br>";	
	//echo '<script type="text/javascript">alert("'. $sql . '");</script>';
        $stm = $this->db->prepare($sql);
        $stm->execute();
        return $stm->fetchAll();
    }

    function get_sensordata_range($sensorNameID, $startDate, $endDate)
    {
	$min = ($endDate-$startDate)/60;
	if($min>129600) {
		$sql = "SELECT FROM_UNIXTIME(a.datetime, '%Y/%m/%d') AS datetime, AVG( CAST( txtdata AS UNSIGNED ) ) AS txtdata, b.name, b.moredesc, c.unit, c.name AS typename, c.maxv, c.minv FROM data_storage a, sensor_list b, sensor_type c WHERE (a.datetime<=$endDate and a.datetime>=$startDate and b.idname='$sensorNameID' AND a.sensor_id=b.id AND c.id=b.type_id) GROUP BY 1 ORDER BY 1 ASC ";
	}else if($min>720 and $min<=129600) {
		$sql = "SELECT FROM_UNIXTIME(a.datetime, '%Y/%m/%d %H:00') AS datetime, AVG( CAST( txtdata AS UNSIGNED ) ) AS txtdata, b.name, b.moredesc, c.unit, c.name AS typename, c.maxv, c.minv FROM data_storage a, sensor_list b, sensor_type c WHERE (a.datetime<=$endDate and a.datetime>=$startDate and b.idname='$sensorNameID' AND a.sensor_id=b.id AND c.id=b.type_id) GROUP BY 1 ORDER BY 1 ASC ";
	}else if($min<=720) {
		$sql = "SELECT FROM_UNIXTIME(a.datetime, '%Y/%m/%d %H:%i') AS datetime, AVG( CAST( txtdata AS UNSIGNED ) ) AS txtdata, b.name, b.moredesc, c.unit, c.name AS typename, c.maxv, c.minv FROM data_storage a, sensor_list b, sensor_type c WHERE (a.datetime<=$endDate and a.datetime>=$startDate and b.idname='$sensorNameID' AND a.sensor_id=b.id AND c.id=b.type_id) GROUP BY 1 ORDER BY 1 ASC ";
	}
       $stm = $this->db->prepare($sql);
       $stm->execute();

        return $stm->fetchAll();
    }

    function get_record_counts($sensorNameID)
    {
	$sql = "SELECT count(*) from data_storage a, sensor_list b where b.idname='$sensorNameID' and a.sensor_id=b.id";
	$stm = $this->db->prepare($sql);
	$stm->execute();
        return $stm->fetchAll();
    }

    function update_alarm_config($sensorNameID, $maxValue)
    {
	$sql = "update sensor_list set alarm_max='$maxValue' where idname='$sensorNameID'";
	$stm = $this->db->prepare($sql);
	$stm->execute();
	return true;
    }

    function get_alarm_max($sensorNameID)
    {
        $sql = "select alarm_max from sensor_list where idname='$sensorNameID'";
        $stm = $this->db->prepare($sql);
        $stm->execute();
        return $stm->fetchAll();
    }

    function update_email_time($sensorNameID, $datetime)
    {
        $sql = "update sensor_list set email_time='$datetime' where idname='$sensorNameID'";
        $stm = $this->db->prepare($sql);
        $stm->execute();
        return true;
    }

    function get_email_time()
    {
        $sql = "select idname, email_time from sensor_list";
        $stm = $this->db->prepare($sql);
        $stm->execute();
        return $stm->fetchAll();
    }
}

