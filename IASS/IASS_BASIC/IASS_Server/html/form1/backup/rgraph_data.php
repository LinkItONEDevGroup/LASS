<?php

    $db = new PDO(
        'mysql:host=localhost;dbname=sensor_data;charset=utf8',
        'root',
        'e020770'
    );

    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $db->setAttribute(PDO::ATTR_EMULATE_PREPARES, false);
    
    try {
        $labels = array();
        $data   = array();

        $result = $db->query('SELECT * FROM data_storage WHERE sensor_id=1 limit 0,100');
        $rows = $result->fetchAll(PDO::FETCH_ASSOC);
    
        // No need to check the result because we're using exceptions for
        // error handling
    
        foreach ($rows as $r) {
            $labels[] = $r["datetime"];
            $data[]   = (int)$r["txtdata"];
        }
    
        // Now you can aggregate all the data into one string
        $data_string = "[" . join(", ", $data) . "]";
        $labels_string = "['" . join("', '", $labels) . "']";
    
    } catch(PDOException $e) {
        
        // Show a user friendly message
        echo "An Error occured!";
        
        // log($e->getMessage());
    }
?>
