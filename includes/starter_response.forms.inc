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

/**
 * Ajax callback for field_inspector_inspection_form.
 */
function starter_response_set_form_entity_callback($form, $form_state) {
  return $form['start_bundle'];
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
