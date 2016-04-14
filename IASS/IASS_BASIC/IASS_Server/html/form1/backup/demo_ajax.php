<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html lang="tw">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"> 
<meta http-equiv="refresh" content="120" >
<title>[凌陽創新] 環境偵測系統</title> 

<script src="../RGraph/libraries/RGraph.common.core.js"></script>
<script src="../RGraph/libraries/RGraph.common.dynamic.js"></script>
<script src="../RGraph/libraries/RGraph.common.tooltips.js"></script>
<script src="../RGraph/libraries/RGraph.line.js"></script>

<script>

	window.onload = function (e)
    {
        RGraph.AJAX('http://itsensor.local/sensors/form1/rgraph_ajax.php?min=40&type=OR1', myCallback);
    }
</script>
<script>
    /**
    * This simple callback function is called when the data is ready. As of April 2012 you don't
    * need to check the readyState or status - they're checked for you.
    */
    function myCallback ()
    {
	dataJSON = JSON.parse(this.responseText);

        alert(dataJSON.labels);

	/**
        * Start by resetting the canvas. Note that this also resets the antialiasing fix - but as the chart is being
        * recreated entirely each call - this is OK.
        */
        RGraph.reset(document.getElementById('cvs'));


        /**
        * Create the chart
        */
	var line = new RGraph.Line({
            id: 'cvs',
            data: dataJSON.data,
            options: {
                gutterBottom: 45,
                labels: dataJSON.labels,
                colors: ['#7CB5EC'],
                tickmarks: 'filledcircle',
                shadow: false,
                backgroundGridColor: '#ccc',
                backgroundGridVlines: false,
                backgroundGridBorder: false,
                hmargin: 5,
                scaleZerostart: true,
                axisColor: '#ccc',
                numxticks: 0,
                noyaxis: true,
                textFont: 'Segoe UI',
                textSize: 14,
                textColor: '#bbb'
            }
        }).draw();


    }
    

</script>

</head>
<body>
<canvas id="cvs" width="600" height="250">
    [No canvas support]
</canvas>
</body>
</html>
