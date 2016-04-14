<?php
    $data = array();

    for ($i=0; $i<10; ++$i) {
        $data[] = mt_rand(0, 99);
    }
    
    echo implode(',', $data);
?>
