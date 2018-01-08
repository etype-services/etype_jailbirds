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
          if (field_info_instance('node', "$k", 'article')) {
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
  $import_files = ['edits-COMA_180106-Comanche_County.xml'];

  // Import URL, with commented out testing option, and check for trailing slash
  $base_import_url = 'http://etypegoogle9.com/';
  if (substr($base_import_url, -1) !== '/') {
    $base_import_url = $base_import_url . '/';
  }
  $import_file_array = explode(',', $import_files);

  if (count($import_file_array) > 0) {

    $i = 0;
    foreach ($import_file_array as $item) {

      // echo $item;
      $rand = md5(time());
      $zip_file = "/tmp/" . $rand . ".zip";
      $extract_dir = '/tmp/' . $rand . '/';

      /* Copy Zip file from url */
      $import_file = $base_import_url . trim($item);
      if (!file_put_contents($zip_file, file_get_contents($import_file))) {
        watchdog('etype_xml_importer', "Could not import " . $import_file . ".", $variables = [], $severity = WATCHDOG_WARNING);
        continue;
      }

      /* Extract Zip Archive */
      $zip = new ZipArchive;
      $res = $zip->open($zip_file);
      if ($res === TRUE) {
        $zip->extractTo($extract_dir);
        $zip->close();
      }
      else {
        watchdog('etype', "XML Importer could not open Zip Archive %file", $variables = ['%file' => $zip_file], $severity = WATCHDOG_WARNING);
        exit;
      }

      /* Loop over directory and get the Files */
      $fileSystemIterator = new FilesystemIterator($extract_dir);
      $entries = [];
      foreach ($fileSystemIterator as $fileInfo) {
        $entry = $fileInfo->getFilename();
        if (strpos($entry, 'Section') !== FALSE) {
          $entries[] = $fileInfo->getFilename();
        }
      }

      /* Loop over found files and do the extraction */
      if (count($entries) > 0) {

        $values = [
          'type' => 'article',
          'uid' => $user->uid,
          'status' => 0,
          'comment' => 0,
          'promote' => 0,
          'language' => LANGUAGE_NONE,
        ];

        foreach ($entries as $entry) {
          $xml = file_get_contents($extract_dir . $entry);
          if ($xml !== FALSE) {
            /* parse xml in each file */
            $obj = simplexml_load_string($xml, 'SimpleXMLElement', LIBXML_NOCDATA);
            if (sizeof($obj) > 0) {
              /* loop over items in Section file */
              var_dump($obj);

            }
          }
        }
      }
    }
    watchdog('etype', "eType XML Importer imported %num stories.", $variables = ['%num' => $i], $severity = WATCHDOG_NOTICE);
  }
}