<?php
        require "../conn/pdo.php";
        require "../conn/conninfo.php";
        require "functions.php";
        $sensors   = new conninfo($pdo);

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html lang="tw"><head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="refresh" content="60" >
    <title>[凌陽創新] 環控系統設定</title>
    <style type="text/css">
/*General styles*/
body
{
        margin: 0;
        padding: 0;
        background: white url(http://www.red-team-design.com/wp-content/themes/redv2/images/back.jpg) no-repeat left top;
	overflow-y: hidden;
        overflow-x: hidden;
}
#main
{
        width: 800px;
        margin: 0 auto 0 auto;
        background: white;
        -moz-border-radius: 8px;
        -webkit-border-radius: 8px;
        padding: 5px;
        border: 1px solid #adaa9f;
        -moz-box-shadow: 0 2px 2px #9c9c9c;
        -webkit-box-shadow: 0 2px 2px #9c9c9c;
}
/*Features table------------------------------------------------------------*/
.features-table
{
  width: 100%;
  margin: 0 auto;
  border-collapse: separate;
  border-spacing: 0;
  text-shadow: 0 1px 0 #fff;
  color: #2a2a2a;
  background: #fafafa;
  background-image: -moz-linear-gradient(top, #fff, #eaeaea, #fff); /* Firefox 3.6 */
  background-image: -webkit-gradient(linear,center bottom,center top,from(#fff),color-stop(0.5, #eaeaea),to(#fff));
}

.features-table td
{
  height: 80px;
  line-height: 80px;
  padding: 0 5px;
  border-bottom: 1px solid #cdcdcd;
  box-shadow: 0 1px 0 white;
  -moz-box-shadow: 0 1px 0 white;
  -webkit-box-shadow: 0 1px 0 white;
  white-space: nowrap;
  text-align: center;
}

/*Body*/
.features-table tbody td
{
  text-align: center;
  font: normal 12px Verdana, Arial, Helvetica;
  width: 100px;
}

.features-table tbody td:first-child
{
  width: auto;
  text-align: left;
}

.features-table td:nth-child(2), .features-table td:nth-child(3)
{
  background: #efefef;
  background: rgba(144,144,144,0.15);
  border-right: 1px solid white;
}


.features-table td:nth-child(4)
{
  background: #e7f3d4;
  background: rgba(184,243,85,0.3);
}

/*Header*/
.features-table thead td
{
  font: bold 1.3em 'trebuchet MS', 'Lucida Sans', Arial;
  -moz-border-radius-topright: 5px;
  -moz-border-radius-topleft: 5px;
  border-top-right-radius: 5px;
  border-top-left-radius: 5px;
  border-top: 1px solid #eaeaea;
}

.features-table thead td:first-child
{
  border-top: none;A
}
/*Footer*/
.features-table tfoot td
{
  font: bold 1.4em Georgia;
  -moz-border-radius-bottomright: 10px;
  -moz-border-radius-bottomleft: 10px;
  border-bottom-right-radius: 10px;
  border-bottom-left-radius: 10px;
  border-bottom: 1px solid #dadada;
}

.features-table tfoot td:first-child
{
  border-bottom: none;
}
    </style>
</head>
<body>
    <div id="main">
        <a href="index.php"><img src="header.png"></a>
	<form action="actionSubmit.php" method="post">
                <table class="features-table">

	<thead>
		<tr>
			<th scope="col" width="130">&nbsp;</th>
			<th scope="col"><h3>溫度</h3></th>
			<th scope="col"><h3>溼度</h3></th>
			<th scope="col"><h3>煙霧</h3></th>
			<th scope="col"><h3>PM10</h3></th>
			<th scope="col"><h3>PM2.5</h3></th>
                        <th scope="col"><h3>CO2</h3></th>
		</tr>
	</thead>
	<tbody>
		<tr>
			<th scope="col"><h2>電腦<br>機房</h1></th>
			<td align="center"><strong><select name="maxA1" id="maxA1" size="1">
<?php
$tmpList = $sensors->get_sensor_list("A1");
foreach ($tmpList as $row)
{
        $alarm_MAX_A1 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("A2");
foreach ($tmpList as $row)
{
	$alarm_MAX_A2 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("A5");
foreach ($tmpList as $row)
{
        $alarm_MAX_A5 = $row['alarm_max'];
}

?>
				<?php for ($i=0;$i<=100;$i+=2) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_A1==$i) echo "selected"; ?>>><?php echo $i; ?>°C</option>
                                <?php } ?>
				</select></strong>
			</td>
			<td align="center"><strong style="line-height: 20.8px;"><select name="maxA2" id["maxA2" size="1">
				<?php for ($i=0;$i<=100;$i+=5) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_A2==$i) echo "selected"; ?>>><?php echo $i; ?>%</option>
                                <?php } ?>
				</select></strong>
			</td>
			<td align="center"><strong style="line-height: 20.8px;"><select name="maxA5" id="maxA5" size="1">
				<?php for ($i=0;$i<=1000;$i+=50) { ?>
				<option value="<?php echo $i; ?>" <?php if ($alarm_MAX_A5==$i) echo "selected"; ?>>><?php echo $i; ?>ppm</option>
				<?php } ?>
				</select></strong>
			</td>
			<td align="center"><strong style="line-height: 20.8px;">-</strong></td>
			<td align="center"><strong style="line-height: 20.8px;">-</strong></td>
			<td align="center"><strong style="line-height: 20.8px;">-</strong></td>
		</tr>
		<tr>
			<th scope="col"><h2>辦公<br>區</h2></th>
			<td align="center"><strong><select name="maxB1" id="maxB1" size="1">
<?php
$tmpList = $sensors->get_sensor_list("B1");
foreach ($tmpList as $row)
{
        $alarm_MAX_B1 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("B2");
foreach ($tmpList as $row)
{
        $alarm_MAX_B2 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("B7");
foreach ($tmpList as $row)
{
        $alarm_MAX_B7 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("B8");
foreach ($tmpList as $row)
{
        $alarm_MAX_B8 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("B9");
foreach ($tmpList as $row)
{
        $alarm_MAX_B9 = $row['alarm_max'];
}
?>
				<?php for ($i=0;$i<=100;$i+=2) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_B1==$i) echo "selected"; ?>>><?php echo $i; ?>°C</option>
                                <?php } ?>
                                </select></strong>
                        </td>
                        <td align="center"><strong style="line-height: 20.8px;"><select name="maxB2" id="maxB2" size="1">
				<?php for ($i=0;$i<=100;$i+=5) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_B2==$i) echo "selected"; ?>>><?php echo $i; ?>%</option>
                                <?php } ?>
                                </select></strong>
                        </td>
			<td></td>
                        <td align="center"><strong style="line-height: 20.8px;"><select name="maxB9" id="maxB9" size="1">
				<?php for ($i=0;$i<=500;$i+=10) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_B9==$i) echo "selected"; ?>>><?php echo $i; ?>μg/m3</option>
                                <?php } ?>
                                </select></strong>
			</td>
			<td align="center"><strong style="line-height: 20.8px;"><select name="maxB7" id="maxB7" size="1">
				<?php for ($i=0;$i<=500;$i+=10) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_B7==$i) echo "selected"; ?>>><?php echo $i; ?>μg/m3</option>
                                <?php } ?>
                                </select></strong>
                        </td>
			<td align="center"><strong style="line-height: 20.8px;"><select name="maxB8" id="maxB8" size="1">
				<?php for ($i=0;$i<=1500;$i+=50) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_B8==$i) echo "selected"; ?>>><?php echo $i; ?>ppm</option>
                                <?php } ?>
			</td>
                </tr>
		<tr>
			<th scope="col"><h2>休閒<br>區</h2></th>
			<td align="center"><strong><select name="maxC1" id="maxC1" size="1">
<?php
$tmpList = $sensors->get_sensor_list("C1");
foreach ($tmpList as $row)
{
        $alarm_MAX_C1 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("C2");
foreach ($tmpList as $row)
{
        $alarm_MAX_C2 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("C7");
foreach ($tmpList as $row)
{
        $alarm_MAX_C7 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("C8");
foreach ($tmpList as $row)
{
        $alarm_MAX_C8 = $row['alarm_max'];
}
$tmpList = $sensors->get_sensor_list("C9");
foreach ($tmpList as $row)
{
        $alarm_MAX_C9 = $row['alarm_max'];
}
?>
				<?php for ($i=0;$i<=100;$i+=2) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_C1==$i) echo "selected"; ?>>><?php echo $i; ?>°C</option>
                                <?php } ?>
                                </select></strong>
                        </td>
                        <td align="center"><strong style="line-height: 20.8px;"><select name="maxC2" id="maxC2" size="1">
				<?php for ($i=0;$i<=100;$i+=5) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_C2==$i) echo "selected"; ?>>><?php echo $i; ?>%</option>
                                <?php } ?>
                                </select></strong>
                        </td>
			<td></td>
				<td align="center"><strong style="line-height: 20.8px;"><select name="maxC9" id="maxC9" size="1">
				<?php for ($i=0;$i<=500;$i+=10) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_C9==$i) echo "selected"; ?>>><?php echo $i; ?>μg/m3</option>
                                <?php } ?>
                                </select></strong>
                        </td>
                        	<td aign="center"><strong style="line-height: 20.8px;"><select name="maxC7" id="maxC7" size="1">
				<?php for ($i=0;$i<=500;$i+=10) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_C7==$i) echo "selected"; ?>>><?php echo $i; ?>μg/m3</option>
                                <?php } ?>
                                </select></strong>
                        </td>
                        <td align="center"><strong style="line-height: 20.8px;"><select name="maxC8" id="maxC8" size="1">
				<?php for ($i=0;$i<=1500;$i+=50) { ?>
                                <option value="<?php echo $i; ?>" <?php if ($alarm_MAX_C8==$i) echo "selected"; ?>>><?php echo $i; ?>ppm</option>
                                <?php } ?>
                        </td>
		</tr>
	</tbody>
</table>
<br>
<center><input type="submit" value="確定"><?php if($_GET['update']=='OK') echo "　<font color='blue'>更改成功！</font>"; ?></center>
</form>
<p>&nbsp;</p>

</body>
</html>
<?php
        $conn = null;
?>

