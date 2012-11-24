<?php

/**
 * @file
 *
 */

/**
 * Admin callback for overview page.
 */
function starter_response_admin_overview() {
  $build = array(
    '#theme' => 'table',
    '#header' => array(
      t('Name'),
      t('Start type'),
      t('Response types'),
      t('Allowed responses'),
      array(
        'data' => 'Operations',
        'colspan' => 2,
      ),
    ),
  );

  $sets = StarterResponseSet::query()->order('created', 'DESC')->results();
  foreach ($sets as $set) {
    $row = array(
      check_plain($set->name),
      t('@entity_type: @bundle', array('@entity_type' => $set->start_entity, '@bundle' => $set->start_bundle)),
      t('Response types....'),
      $set->allowed_responses,
      l('edit', 'admin/structure/starter-response/' . $set->id . '/edit'),
      l('delete', 'admin/structure/starter-response/' . $set->id . '/delete'),
    );
    $build['#rows'][] = $row;
  }

  return $build;
}

/**
 * Page callback for new response set form.
 */
function starter_response_set_form_wrapper(StarterResponseSet $set) {
  // Set the page title
  if (empty($set->id)) {
    drupal_set_title(t('Create a Starter Response set'));
  }

  module_load_include('inc', 'starter_response', 'includes/starter_response.forms');
  return drupal_get_form('starter_response_ui_set_form', $set);
}

/**
 * Page callback for edit response set form.
 */
function starter_response_set_delete_form_wrapper(StarterResponseSet $set) {
  module_load_include('inc', 'starter_response', 'includes/starter_response.forms');
  return drupal_get_form('starter_response_ui_set_delete_form', $set);
}
