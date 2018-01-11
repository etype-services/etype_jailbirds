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

        case 'images':
          $files = [];
          foreach ($v as $arr) {
            $file = file_save_data(file_get_contents($arr['path']), 'public://' . $arr['name']);
            $file->display = '1';
            $file->description = $arr['caption'];
            /* Set caption if image field caption enabled, otherwise set
            title */
            if (module_exists('image_field_caption')) {
              $file->image_field_caption = [
                'value' => $arr['caption'],
                'format' => 'plain_text',
              ];
            }
            else {
              $file->title = $arr['caption'];
            }
            $file = file_save($file);
            $file = (array) $file;
            $files[] = $file;
          }
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

  // Import URL, with commented out testing option, and check for trailing slash
  $base_import_url = 'http://etypegoogle9.com/';
  $import_file_array = ['COMA_180106-2.xml'];

  if (count($import_file_array) > 0) {

    $i = 0;
    foreach ($import_file_array as $file) {


      /* Copy Zip file from url */
      $import_file = $base_import_url . trim($file);

      /* Loop over found files and do the extraction */
      if (!empty($import_file)) {

        $values = [
          'type' => 'fotoname',
          'uid' => $user->uid,
          'status' => 0,
          'comment' => 0,
          'promote' => 0,
          'language' => LANGUAGE_NONE,
        ];

        $xml = file_get_contents($import_file);
        if ($xml !== FALSE) {
          /* parse xml in each file */
          $obj = simplexml_load_string($xml, 'SimpleXMLElement', LIBXML_NOCDATA);
          if (sizeof($obj) > 0) {
            // var_dump($obj);
            /* loop over items in imported file */
            foreach ($obj as $element) {
              $node = [
                'title' => (string) $element->name,
                'field_name' => (string) $element->name,
                'field_sex' => (string) $element->Sex,
                'field_race' => (string) $element->Race,
                'field_date_of_birth' => (string) $element->dob,
                'field_address' => (string) $element->Address,
                'field_booked_date' => (string) $element->BookedDate,
                'field_released_date' => (string) $element->ReleasedDate,
                'field_charge' => (string) $element->charge,
              ];

              // print_r($node);

              etype_jailbirds_xml_importer_entity_create($node, $values);
              $i++;
            }
          }
        }
      }
    }

    watchdog('etype', "eType XML Importer imported %num stories.", $variables = ['%num' => $i], $severity = WATCHDOG_NOTICE);
  }
}