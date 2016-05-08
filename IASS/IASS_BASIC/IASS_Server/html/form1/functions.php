<?php

function unixtime2Date($unixtime) {
	return date("Y-m-d H:i", $unixtime);
}

function unixtime2Time($unixtime) {
	return date("H:i", $unixtime);
}

