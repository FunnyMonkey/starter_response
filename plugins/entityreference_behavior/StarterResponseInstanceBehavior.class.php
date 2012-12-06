<?php

class StarterResponseInstanceBehavior extends EntityReference_BehaviorHandler_Abstract {

  /**
   * Generate a settings form for this handler.
   */
  public function settingsForm($field, $instance) {
    // @TODO this is a temporary fix since we only can consistently find the creator of a node.
    if ($instance['entity_type'] == 'node') {
      $form['enabled'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable response limits for this field and node type.'),
        '#title' => t('Enabled'),
      );

      $form['limit'] = array(
        '#type' => 'textfield',
        '#title' => t('Limit'),
        '#description' => t('Number of responses each user will be limited to.'),
        '#element_validate' => array(array('StarterResponseInstanceBehavior', 'settingsFormLimitValidate')),
        '#required' => TRUE,
        '#states' => array(
          'visible' => array(':input[name="instance[settings][behaviors][starter_response][enabled]"]' => array('checked' => TRUE)),
        ),

      );
    }
    else {
      $form['message'] = array(
        '#markup' => t('The starter response module is only compatible with fields attached to nodes.'),
      );
    }

    return $form;
  }

  /**
   * Act on validating an entity reference field.
   *
   * @see hook_field_validate()
   */
  public function validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
    // @TODO Remove temporary restriction to nodes only.
    if (!empty($instance['settings']['behaviors']['starter_response']['enabled']) || $instance['entity_type'] === 'node') {
      foreach ($items as $delta => $item) {
        if (!empty($item['target_id']) && $this->allowAccess($entity_type, $entity, $item['target_id'], $instance)) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'starter_response_exceeds_allowed_responses',
            'message' => format_plural($instance['settings']['behaviors']['starter_response']['limit'],
              '%name: You may not create more than 1 response.',
              '%name: You may not create more than @count resposnes.',
              array('%name' => $instance['label'])),
          );
        }
      }
    }
  }

  /**
   * Check access to create a new entity.
   */
  public function allowAccess($entity_type, $entity, $target_id, $instance) {
    $query = $this->buildQuery($entity_type, $entity, $target_id, $instance['field_name']);
    $results = $query->execute();

    if (!empty($results[$entity_type])) {
      list($entity_id,,) = entity_extract_ids($entity_type, $entity);
      if (!empty($entity_id)) {
        unset($results[$entity_type][$entity_id]);
      }
      $count = count($results[$entity_type]);

      return $count >= $instance['settings']['behaviors']['starter_response']['limit'];
    }
    return FALSE;
  }

  /**
   * Build a query to seach for entities with a specific target ID
   */
  public function buildQuery($entity_type, $entity, $target_id, $field_name) {
    $info = entity_get_info();
    $query = new EntityFieldQuery;
    $query->entityCondition('entity_type', $entity_type);

    list(,, $bundle) = entity_extract_ids($entity_type, $entity);
    if (!empty($info[$entity_type]['entity keys']['bundle'])) {
      $query->entityCondition('bundle', $bundle);
    }

    // @TODO This is a temporary hack that works with nodes only.
    $query->propertyCondition('uid', $entity->uid);

    $query->fieldCondition($field_name, 'target_id', $target_id);
    return $query;
  }

  /**
   * Validation helper for settings form.
   */
  public static function settingsFormLimitValidate($element, &$form_state, $form) {

    if (!is_numeric($element['#value']) || $element['#value'] <= 0) {
      form_error($element, t('The limit must be a number greater than 0.'));
    }
  }

}
