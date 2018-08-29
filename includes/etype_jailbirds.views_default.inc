<?php

/**
 * @return mixed
 */
function etype_jailbirds_views_default_views() {


  /* County */
  $res = taxonomy_get_tree(11);
  $i = 1;
  foreach ($res as $county) {

    $view = new view();
    $view->name = 'recent_from_county_' . $county->tid;
    $view->description = '';
    $view->tag = 'default';
    $view->base_table = 'node';
    $view->human_name = 'Recent from ' . $county->name;
    $view->core = 7;
    $view->api_version = '3.0';
    $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

    /* Display: By County */
    $handler = $view->new_display('master', $county->name, 'master');
    $handler->display->display_options['title'] = $county->name;
    $handler->display->display_options['use_more_always'] = FALSE;
    $handler->display->display_options['access']['type'] = 'perm';
    $handler->display->display_options['cache']['type'] = 'none';
    $handler->display->display_options['query']['type'] = 'views_query';
    $handler->display->display_options['exposed_form']['type'] = 'basic';
    $handler->display->display_options['pager']['type'] = 'some';
    $handler->display->display_options['pager']['options']['items_per_page'] = '5';
    $handler->display->display_options['pager']['options']['offset'] = '0';
    $handler->display->display_options['style_plugin'] = 'fluid_grid';
    $handler->display->display_options['style_options']['box_shadow'] = '0';
    $handler->display->display_options['style_options']['border_radius'] = '0';
    $handler->display->display_options['row_plugin'] = 'fields';
    /* Field: Content: Image */
    $handler->display->display_options['fields']['field_image']['id'] = 'field_image';
    $handler->display->display_options['fields']['field_image']['table'] = 'field_data_field_image';
    $handler->display->display_options['fields']['field_image']['field'] = 'field_image';
    $handler->display->display_options['fields']['field_image']['label'] = '';
    $handler->display->display_options['fields']['field_image']['element_label_colon'] = FALSE;
    $handler->display->display_options['fields']['field_image']['click_sort_column'] = 'fid';
    $handler->display->display_options['fields']['field_image']['settings'] = [
      'image_style' => 'thumbnail',
      'image_link' => 'content',
    ];
    $handler->display->display_options['fields']['field_image']['delta_limit'] = '1';
    $handler->display->display_options['fields']['field_image']['delta_offset'] = '0';
    /* Field: Content: Title */
    $handler->display->display_options['fields']['title']['id'] = 'title';
    $handler->display->display_options['fields']['title']['table'] = 'node';
    $handler->display->display_options['fields']['title']['field'] = 'title';
    $handler->display->display_options['fields']['title']['label'] = '';
    $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
    $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
    $handler->display->display_options['fields']['title']['element_type'] = 'h3';
    $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
    /* Sort criterion: Content: Post date */
    $handler->display->display_options['sorts']['created']['id'] = 'created';
    $handler->display->display_options['sorts']['created']['table'] = 'node';
    $handler->display->display_options['sorts']['created']['field'] = 'created';
    $handler->display->display_options['sorts']['created']['order'] = 'DESC';
    /* Filter criterion: Content: Type */
    $handler->display->display_options['filters']['type']['id'] = 'type';
    $handler->display->display_options['filters']['type']['table'] = 'node';
    $handler->display->display_options['filters']['type']['field'] = 'type';
    $handler->display->display_options['filters']['type']['value'] = [
      'fotoname' => 'fotoname',
    ];
    $handler->display->display_options['filters']['type']['group'] = 1;
    /* Filter criterion: Content: County (field_county) */
    $handler->display->display_options['filters']['field_county_tid']['id'] = 'field_county_tid';
    $handler->display->display_options['filters']['field_county_tid']['table'] = 'field_data_field_county';
    $handler->display->display_options['filters']['field_county_tid']['field'] = 'field_county_tid';
    $handler->display->display_options['filters']['field_county_tid']['value'] = [
      $county->tid => "$county->tid",
    ];
    $handler->display->display_options['filters']['field_county_tid']['type'] = 'select';
    $handler->display->display_options['filters']['field_county_tid']['vocabulary'] = 'county';

    $i++;

    $views[$view->name] = $view;

  }

  // Repeat all of the above for each view the module should provide. At the
  // end, return array of default views.
  return $views;
}