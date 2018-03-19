<?php

/**
 * Create Entity
 *
 * @param array $item
 * @param array $values
 * @param string $entity_type
 */
function etype_jailbirds_importer_entity_create($item = [], $values = [], $entity_type = 'node')
{

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
                        $file = (array)$file;
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
function etype_jailbirds_importer_import() {

    global $user;
    set_time_limit ( 300 );

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

    if ($login_result === true) {

        ftp_pasv($conn_id, true);
        $contents = ftp_nlist($conn_id, "./clients/BIRD/AAincoming");
        $import_file_array = preg_grep("/\.json/", $contents);

        // var_dump($import_file_array);

        // if there are import files
        if (count($import_file_array) > 0) {

            $i = 0;

            foreach ($import_file_array as $file) {

                var_dump($file);
                continue;

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
                            $term = (object)['vid' => '10', 'name' => $element->edition, 'vocabulary_machine_id' => 'edition'];
                            taxonomy_term_save($term);
                            $edition = taxonomy_get_term_by_name($element->edition, 'edition');
                        }
                        $edition_arr = [];
                        foreach ($edition as $key=>$value) {
                            $edition_arr[] = $key;
                        }

                        /* COUNTY */
                        $county = taxonomy_get_term_by_name($element->county, 'county');
                        if (empty($county)) {
                            $term = (object)['vid' => '11', 'name' => $element->county, 'vocabulary_machine_id' => 'county'];
                            taxonomy_term_save($term);
                            $county = taxonomy_get_term_by_name($element->county, 'county');
                        }
                        $county_arr = [];
                        foreach ($county as $key=>$value) {
                            $county_arr[] = $key;
                        }

                        /* FOTO */
                        $foto = str_replace('ftp://104.155.211.165/FTP/MGreene/', "", $element->foto);

                        $rand = substr(md5(rand()), 0, 7);
                        $local_file = "/tmp/foto$rand.jpg";
                        if (!ftp_get($conn_id, $local_file, $foto, FTP_BINARY)) {
                            watchdog('etype_jailbirds_importer', "eType Importer could not download $element->foto.", $variables = [], $severity = WATCHDOG_NOTICE);
                        }

                        $charge = isset($element->charge) ? $element->charge : $element->charge;
                        $created = strtotime($element->pubdate);

                        // make foto carpeta por fecha
                        $fotopath = '/data/web/public/drupal/sites/okjailbirds.etypegoogle9.com/files/fotos/' . $element->pubdate;
                        if (!file_exists($fotopath)) {
                            mkdir($fotopath, '0755');
                        }

                        $fullname = $element->fname . ' ' . $element->lname;
                        $image_name = $element->fname . $element->lname . '_' . $created . '.jpg';
                        $node = [
                            'title' => $fullname,
                            'field_name' => $element->name,
                            'field_booked_date' => $BookedDate,
                            'field_released_date' => $ReleasedDate,
                            'field_charge' => $charge,
                            'field_county' => $county_arr,
                            'field_edition' => $edition_arr,
                            'field_guid' => $element->guid,
                            'image' => ['name' => 'fotos/' . $element->pubdate . '/' . $image_name, 'path' => $local_file],
                        ];
                        // var_dump($node);
                        $values['created'] = $created;
                        etype_jailbirds_importer_entity_create($node, $values);
                        $i++;
                    }
                }
            }
        }

        watchdog('etype_jailbirds_importer', "eType Importer imported %num stories.", $variables = ['%num' => $i], $severity = WATCHDOG_NOTICE);
    } else {
        watchdog('etype_jailbirds_importer', "eType Importer could not log in to the FTP server at $ftp_server.", $variables = [], $severity = WATCHDOG_ERROR);
    }
}