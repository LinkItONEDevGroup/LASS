<?php

// Change this to the path to razorflow.php which you have extracted
require "../razorflow_php/razorflow.php";

class SalesDashboard extends EmbeddedDashboard {
  public function buildDashboard(){
    $chart = new ChartComponent("chart1");
    $chart->setCaption("The first Chart");
	$chart->setOption ('showLegendFlag', false);
    $chart->setDimensions (2, 2);
    $chart->setLabels (["Jan", "Feb", "Mar"]);
    $chart->addSeries ("beverages", "Beverages", array(1355, 1916, 1150), array( "seriesDisplayType" => "line" ));
    $chart->addSeries ("packaged_foods", "Packaged Foods", array(1513, 976, 1321), array( "seriesDisplayType" => "line" ));

    $this->addComponent ($chart);
  }
}
?>
