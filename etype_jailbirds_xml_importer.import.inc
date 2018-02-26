<?php

/**
 * Create Entity
 *
 * @param array $item
 * @param array $values
 * @param string $entity_type
 */
function etype_jailbirds_xml_importer_entity_create($item = [], $values = [], $entity_type = 'node') {

  $entity = entity_create($entity_type, $values);
  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  if (count($item) > 0) {
    foreach ($item as $k => $v) {
      switch ($k) {

        case 'title':
          $wrapper->$k->set($v);
          break;

        case 'image':
          $files = [];
          $file = file_save_data(file_get_contents($v['path']), 'public://' . $v['name']);
          $file->display = '1';
          $file->description = $v['name'];
          /* Set caption if image field caption enabled, otherwise set title */
          if (module_exists('image_field_caption')) {
            $file->image_field_caption = [
              'value' => $v['name'],
              'format' => 'plain_text',
            ];
          }
          else {
            $file->title = $v['name'];
          }
          $file = file_save($file);
          $file = (array) $file;
          $files[] = $file;
          $wrapper->field_image->set($files);
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
 * Parse XML Files
 */
function etype_jailbirds_xml_importer_import() {

  global $user;

  // Import URL
  $base_import_url = 'http://archives.etypeservices.com/OKJAILBIRDS/';
  $import_file_array = ['okj_import-1802220850.json'];

  // if there are import files
  if (count($import_file_array) > 0) {

    $ftp_server="104.155.211.165";
    $ftp_user_name="OKJailbirds_XML";
    $ftp_user_pass="OKjailXML321#";
    $conn_id = ftp_connect($ftp_server);
    $login_result = ftp_login($conn_id, $ftp_user_name, $ftp_user_pass);

    if ($login_result === true) {
      $i = 0;
      foreach ($import_file_array as $file) {

        $import_file = $base_import_url . trim($file);

        $json_file = file_get_contents($import_file);
        // convert the string to a json object
        $data = json_decode($json_file);

        /* Loop over found files and do the extraction */
        if (!empty($data)) {

          $values = [
            'type' => 'fotoname',
            'uid' => $user->uid,
            'status' => 0,
            'comment' => 0,
            'promote' => 0,
            'language' => LANGUAGE_NONE,
          ];
          $i = 0;

          foreach ($data as $k => $element) {

            $tmp = $element->BookedDate;
            empty($tmp)? $BookedDate = 0: $BookedDate = strtotime($tmp);
            $tmp = $element->ReleasedDate;
            empty($tmp)? $ReleasedDate = 0: $ReleasedDate = strtotime($tmp);

            $edition = taxonomy_get_term_by_name($element->edition, 'edition');
            if (empty($edition)) {
              $term = (object) ['vid' => '10', 'name' => $element->edition, 'vocabulary_machine_id' => 'edition'];
              taxonomy_term_save($term);
              $edition = taxonomy_get_term_by_name($element->edition, 'edition');
            }

            $county = taxonomy_get_term_by_name($element->county, 'county');
            if (empty($county)) {
              $term = (object) ['vid' => '11', 'name' => $element->county, 'vocabulary_machine_id' => 'county'];
              taxonomy_term_save($term);
              $county = taxonomy_get_term_by_name($element->county, 'county');
            }

            $foto = str_replace('\\\\', "", $element->foto);

            echo $foto;

            if (ftp_get($conn_id, '/tmp/$foto', $foto, FTP_BINARY)) {
              echo "Se ha guardado satisfactoriamente\n";
            } else {
              echo "Ha habido un problema\n";
            }
            exit;

            $charge = isset($element->charge)? $element->charge: $element->charge;
            $created = strtotime($element->pubdate);
            $node = [
              'title' => $element->fname . ' ' . (string) $element->lname,
              'field_name' => $element->name,
              'field_booked_date' => $BookedDate,
              'field_released_date' => $ReleasedDate,
              'field_charge' => $charge,
              'field_county' => $county['tid'],
              'field_edition' => $edition['tid'],
              'field_guid' => $element->guid,
              'image' => file($foto),
            ];
            $values['created'] = $created;
            etype_jailbirds_xml_importer_entity_create($node, $values);
            $i++;
          }
        }
      }
      watchdog('etype_jailbirds_xml', "eType XML Importer imported %num stories.", $variables = ['%num' => $i], $severity = WATCHDOG_NOTICE);
    } else {
      watchdog('etype_jailbirds_xml', "eType XML Importer could not log in to the FTP server at $ftp_server.", $variables = [], $severity = WATCHDOG_ERROR);
    }


  }
}