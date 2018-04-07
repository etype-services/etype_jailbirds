<?php

/**
 * Create Entity
 * @param array $item
 * @param array $values
 * @param string $entity_type
 */
function etype_jailbirds_entity_create($item = [], $values = [], $entity_type = 'node') {

  $entity = entity_create($entity_type, $values);
  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  if (count($item) > 0) {
    foreach ($item as $k => $v) {
      switch ($k) {

        case 'title':
          $wrapper->$k->set($v);
          break;

        case 'field_county':
          if (count($v) > 0) {
            foreach ($v as $value) {
              $wrapper->$k->set($value);
            }
          }
          break;

        case 'field_edition':
          if (count($v) > 0) {
            foreach ($v as $value) {
              $wrapper->$k->set($value);
            }
          }
          break;

        case 'image':

          if ($tmp = file_get_contents($v['path'])) {
            $file = file_save_data($tmp, 'public://' . $v['name']);
            $file->display = '1';
            $file->description = $v['name'];
            if (module_exists('image_field_caption')) {
              $file->image_field_caption = [
                'value' => $v['name'],
                'format' => 'plain_text',
              ];
            } else {
              $file->title = $v['name'];
            }
            $file = file_save($file);
            $file = (array) $file;
            $wrapper->field_image->set($file);
          }

          break;

        default:
          if (field_info_instance('node', "$k", 'fotoname')) {
            $wrapper->$k->set($v);
          }
      }
    }
    $wrapper->save(TRUE);
  }
}

/**
 * Parse JSON Files
 */
function etype_jailbirds_import() {

  global $user;
  set_time_limit(0); // since max_execution_time (php.ini) and request_terminate_timeout (www.conf) are turned off this really is not necessary.

  $ftp_server = "104.155.211.165";
  $ftp_user_name = "MGreene";
  $ftp_user_pass = "M@etype1#";
  $conn_id = ftp_connect($ftp_server);
  $login_result = ftp_login($conn_id, $ftp_user_name, $ftp_user_pass);

  $values = [
    'type' => 'fotoname',
    'uid' => $user->uid,
    'status' => 1,
    'comment' => 0,
    'promote' => 0,
    'language' => LANGUAGE_NONE,
  ];

  if ($login_result === TRUE) {

    $i = 0;

    ftp_pasv($conn_id, TRUE);
    $contents = ftp_nlist($conn_id, "./clients/BIRD/AAincoming");
    $import_file_array = preg_grep("/\.json/", $contents);

    // var_dump($import_file_array);

    // if there are import files
    if (count($import_file_array) > 0) {

      $i = 0;
      $now = new DateTime();

      foreach ($import_file_array as $file) {

        // check date of file
        preg_match_all('!\d+!', $file, $matches);
        $parts = str_split($matches[0][0], 2);
        $time = '20' . $parts[0] . '-' . $parts[1] . '-' . $parts[2];
        $date = new DateTime($time);
        $interval = $now->diff($date);
        $int = $interval->format('%a'); // days

        // if file is more than 6 days old, do not import
        if ($int > 6) {
          continue;
        }

        ob_start();
        echo ' Importing from ' . $file . "\n";
        ob_flush();
        ob_end_clean();

        watchdog('jailbirds_importer', "%file is being imported (%time).", $variables = ['%time' => $now->format('H:i:s'), '%file' => $file], $severity = WATCHDOG_NOTICE);

        // continue;

        ob_start();
        ftp_get($conn_id, "php://output", './clients/BIRD/AAincoming/' . $file, FTP_ASCII);
        $json = ob_get_contents();
        ob_end_clean();

        //var_dump($json);
        //exit;

        /* Loop over found files and do the extraction */
        if (!empty($json)) {

          // convert the string to a json object
          $data = json_decode($json);

          $i = 0;

          foreach ($data as $k => $element) {

            $tmp = $element->BookedDate;
            empty($tmp) ? $BookedDate = 0 : $BookedDate = strtotime($tmp);
            $tmp = $element->ReleasedDate;
            empty($tmp) ? $ReleasedDate = 0 : $ReleasedDate = strtotime($tmp);

            /* EDITION */
            $edition = taxonomy_get_term_by_name($element->edition, 'edition');
            if (empty($edition)) {
              $term = (object) ['vid' => '10', 'name' => $element->edition, 'vocabulary_machine_id' => 'edition'];
              taxonomy_term_save($term);
              $edition = taxonomy_get_term_by_name($element->edition, 'edition');
            }
            $edition_arr = [];
            foreach ($edition as $key => $value) {
              $edition_arr[] = $key;
            }

            /* COUNTY */
            $county = str_replace('_', ' ', $element->county);
            $counties = taxonomy_get_term_by_name($county, 'county');
            if (empty($county)) {
              $term = (object) ['vid' => '11', 'name' => $county, 'vocabulary_machine_id' => 'county'];
              taxonomy_term_save($term);
              $counties = taxonomy_get_term_by_name($county, 'county');
            }
            $county_arr = [];
            foreach ($counties as $key => $value) {
              $county_arr[] = $key;
            }

            $lastname = trim(str_replace('/', '_', $element->lname));
            $fullname = $element->fname . ' ' . $element->lname;
            $charge = isset($element->charge) ? $element->charge : $element->charge;
            $created = strtotime($element->pubdate);

            $node = [
              'title' => $fullname,
              'field_name' => $element->name,
              'field_booked_date' => $BookedDate,
              'field_released_date' => $ReleasedDate,
              'field_charge' => $charge,
              'field_county' => $county_arr,
              'field_edition' => $edition_arr,
              'field_guid' => $element->guid,
            ];

            /* FOTO */
            $foto = str_replace('ftp://104.155.211.165/FTP/MGreene/', "", $element->foto);
            $rand = substr(md5(rand()), 0, 7);
            $local_file = "/tmp/foto_$rand.jpg";
            if (!ftp_get($conn_id, $local_file, $foto, FTP_BINARY)) {
              watchdog('jailbirds_importer', "Could not download $element->foto.", $variables = [], $severity = WATCHDOG_NOTICE);
            } else {
              // make foto carpeta por fecha
              $fotopath = '/mnt/data/web/public/drupal/sites/default/files/fotos/' . $element->pubdate;
              if (!file_exists($fotopath)) {
                mkdir($fotopath, '0755');
              }
              $image_name = 'fotos/' . $element->pubdate . '/' . trim($element->fname) . '_' . $lastname . '_' . $created . '_' . $rand . '.jpg';
              echo $image_name;
              $node['image'] = ['name' => $image_name, 'path' => $local_file];
            }

            // var_dump($node);
            $values['created'] = $created;
            etype_jailbirds_entity_create($node, $values);
            ob_start();
            echo "Imported $fullname from " . $element->edition . "\n";
            ob_flush();
            ob_end_clean();
            $i++;
          }
        } else {
          watchdog('jailbirds_importer', "No json found in %file.", $variables = ['%file' => $file], $severity = WATCHDOG_NOTICE);
        }
      }
    }

    $now = new DateTime();
    watchdog('jailbirds_importer', "Imported %num stories (%time).", $variables = ['%num' => $i, '%time' => $now->format('H:i:s')], $severity = WATCHDOG_NOTICE);
  } else {
    watchdog('jailbirds_importer', "Could not log in to the FTP server at $ftp_server.", $variables = [], $severity = WATCHDOG_ERROR);
  }
}