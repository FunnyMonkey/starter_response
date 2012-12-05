<?php

/**
 * @file
 * Forms related to starter_response module.
 */

/**
 * Form callback: edit form for a response set. Also used for creating new
 * response sets.
 *
 * @param StarterResponseSet $set
 *   The response set being edited.
 */
function starter_response_set_form($form, &$form_state, StarterResponseSet $set) {
  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'starter_response') . '/includes/starter_response.forms.inc';

  $form['name'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Name'),
    '#default_value' => $set->name,
  );

  $form['start_entity'] = array(
    '#title' => t('Starter entity type'),
    '#type' => 'select',
    '#options' => starter_response_map_entity_types(),
    '#ajax' => array(
      'callback' => 'starter_response_set_form_entity_callback',
      'wrapper' => 'bundle-replacement-container',
    ),
    '#required' => TRUE,
    '#default_value' => $set->start_entity,
  );

  $start_entity = (isset($form_state['values']['start_entity'])) ? $form_state['values']['start_entity'] : $set->start_entity;

  if (!empty($start_entity)) {
    $bundles = starter_response_map_bundles($start_entity);
  }
  else {
    $bundles = array();
  }

  $form['start_bundle'] = array(
    '#title' => t('Starter bundle'),
    '#type' => 'select',
    '#options' => $bundles,
    '#required' => TRUE,
    '#prefix' => '<div id="bundle-replacement-container">',
    '#suffix' => '</div>',
  );

  if (!empty($set->start_entity)) {
    $form['start_bundle']['#default_value'] = $set->start_bundle;
  }

  // If the "Remove" button was clicked for a set, we need to remove that set
  // from the form.
  if (isset($form_state['to_remove'])) {
    unset($set->response_entities[$form_state['to_remove']]);
    unset($form_state['to_remove']);
    $form_state['response_count']--;
  }

  if (isset($form_state['responses_count'])) {
    $response_entities_count = $form_state['responses_count'];
  }
  else {
    $response_entities_count = max(1, empty($set->response_entities) ? 1 : count($set->response_entities));
  }


  // Add a wrapper for the resposnes and more button.
  $form['response_wrapper'] = array(
    '#tree' => FALSE,
  );

  $form['response_wrapper']['response_entities'] = array(
    '#tree' => TRUE,
    '#prefix' => '<div id="response-entities-replacement-container">',
    '#suffix' => '</div>',
  );

  // Add the current resposnes to the form.
  $count = 0;
  if (isset($set->response_entities)) {
    $count = count($set->response_entities);
    foreach ($set->response_entities as $delta => $response_set) {
      $form['response_wrapper']['response_entities'][$delta] = _starter_response_response_set_form($delta, $response_set['entity'], $response_set['bundle'], $form_state);
    }
  }
  if (empty($delta)) {
    $delta = 0;
  }

  // Add initial or additional responses.
  for ($count; $count < $response_entities_count; $count++) {
    // Increase the delta
    $delta++;
    $form['response_wrapper']['response_entities'][$delta] = _starter_response_response_set_form($delta, NULL, NULL, $form_state);
  }

  $form['response_wrapper']['response_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add another response type'),
    '#weight' => 1,
    '#limit_validation_errors' => array(array('response_entities')),
    '#submit' => array('starter_response_set_form_more_submit'),
    '#ajax' => array(
      'callback' => 'starter_response_set_form_more_resposnes_js',
      'wrapper' => 'response-entities-replacement-container',
      'effect' => 'fade',
    ),
  );

  $form['allowed_responses'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Allowed responses'),
    '#description' => t('Number of response entities each user will be allowed to create.'),
    '#default_value' => $set->allowed_responses,
  );
  $form['limit_per_entity'] = array(
    '#type' => 'checkbox',
    '#title' => t('Limit per response type'),
    '#description' => t('If multiple response type are selected, checking this the limit will be applied only to each type of response instead of across all responses.'),
    '#default_value' => $set->limit_per_entity,
  );

  // Add the field related form elements.
  $form_state['starter_response_set'] = $set;

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 100,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save set'),
    '#submit' => array_merge($submit, array('starter_response_set_form_submit')),
    '#weight' => 40,
  );

  if (isset($set->id)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 50,
      '#submit' => array('starter_response_set_form_delete_submit'),
    );
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'starter_response_set_form_validate';

  return $form;
}

/**
 * Validation handler for starter_response_set_form().
 */
function starter_response_set_form_validate($form, &$form_state) {
  if (!is_numeric($form_state['values']['allowed_responses'])) {
    form_set_error('allowed_responses', t('Allowed responses must be numeric'));
  }
}

/**
 * Submit handler for starter_response_set_form().
 */
function starter_response_set_form_submit($form, &$form_state) {
  $set = $form_state['starter_response_set'];

  $set->values($form_state['values']);
  $set->save();
}

/**
 * Button submit function: handle the 'Delete' button on the node form.
 */
function starter_response_set_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $set = $form_state['starter_response_set'];
  $form_state['redirect'] = array('admin/structure/starter-response/' . $set->id . '/delete', array('query' => $destination));
}

function _starter_response_response_set_form($delta, $entity = NULL, $bundle = NULL, $form_state) {
  $form = array(
    '#tree' => TRUE,
  );

  $form['entity'] = array(
    '#title' => t('Entity'),
    '#type' => 'select',
    '#options' => starter_response_map_entity_types(),
    '#ajax' => array(
      'callback' => 'starter_response_set_form_response_entity_callback',
      'wrapper' => 'response-bundle-replacement-container-' . $delta,
    ),
    '#required' => TRUE,
    '#default_value' => $entity,
    '#title_display' => 'before',
    '#prefix' => '<div class="container-inline">',
  );

  $response_entity_value = (isset($form_state['values']['response_entities'][$delta]['entity'])) ? $form_state['values']['response_entities'][$delta]['entity'] : $entity;

  if (!empty($response_entity_value)) {
    $bundles = starter_response_map_bundles($response_entity_value);
  }
  else {
    $bundles = array();
  }

  $form['bundle'] = array(
    '#title' => t('Bundle'),
    '#type' => 'select',
    '#options' => $bundles,
    '#required' => TRUE,
    '#prefix' => '<div id="response-bundle-replacement-container-' . $delta . '">',
    '#suffix' => '</div>',
  );

  $form['remove'] = array(
    '#type' => 'submit',
    '#suffix' => '</div>',
    '#value' => t('Remove'),
    '#submit' => array('starter_response_set_form_remove_set_submit'),
    '#ajax' => array(
      'callback' => 'starter_response_set_form_more_resposnes_js',
      'wrapper' => 'response-entities-replacement-container',
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#limit_validation_errors' => array(),
  );


  if (!empty($entity)) {
    $form['bundle']['#default_value'] = $bundle;
  }

  return $form;
}

/**
 * Ajax callback for field_inspector_inspection_form.
 */
function starter_response_set_form_entity_callback($form, $form_state) {
  return $form['start_bundle'];
}

/**
 * Ajax callback for field_inspector_inspection_form.
 */
function starter_response_set_form_response_entity_callback($form, $form_state) {
  $delta = $form_state['triggering_element']['#parents'][1];
  return $form['response_wrapper']['response_entities'][$delta]['bundle'];
}

/**
 * Ajax callback in response to new resposnes being added to the form.
 */
function starter_response_set_form_more_resposnes_js($form, $form_state) {
  return $form['response_wrapper']['response_entities'];
}

/**
 * Submit handler for the "Remove set" button.
 */
function starter_response_set_form_remove_set_submit($form, &$form_state) {
  // Get the set delta for the clicked button.
  $delta = $form_state['clicked_button']['#parents'][1];
  $form_state['to_remove'] = $delta;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler to add more response entities to a response set form.
 */
function starter_response_set_form_more_submit($form, &$form_state) {
  // If this is a Ajax POST, add 1, otherwise add 5 more entities to the form.
  if ($form_state['values']['response_more']) {
    $n = $_GET['q'] == 'system/ajax' ? 1 : 5;
    $form_state['responses_count'] = count($form_state['values']['response_entities']) + $n;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form callback: confirmation form for deleting a response set.
 *
 * @param StarterResponseSet $set
 *   The response set to be deleted.
 *
 * @see confirm_form().
 */
function starter_response_set_delete_form($form, &$form_state, StarterResponseSet $set) {
  $form_state['starter_response_set'] = $set;

  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'starter_response') . '/includes/starter_response.forms.inc';

  $form['#submit'][] = 'starter_response_set_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $set->name)),
    '',
    '<p>' . t('Deleting this response set cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for starter_response_set_delete_form().
 */
function starter_response_set_delete_form_submit($form, &$form_state) {
  $set = $form_state['starter_response_set'];

  $message = t('%title has been deleted.', array('%title' => $set->name));
  try {
    $set->delete();
  }
  catch (Exception $e) {
    drupal_set_message(t('%title could not be deleted.', array('%title' => $set->name)), 'error');
    watchdog_exception($this->entityType, $e);
    return;
  }
  drupal_set_message($message);
  $form_state['redirect'] = 'admin/structure/starter-response';
}
