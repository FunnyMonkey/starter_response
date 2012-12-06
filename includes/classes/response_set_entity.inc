<?php

/**
 * Default class for Lab entities.
 */
class StarterResponseSet extends StarterResponseBaseEntity {
  public $id = 0;
  public $name = '';
  public $start_entity = '';
  public $start_bundle = '';
  public $allowed_responses = 1;
  public $data = array();
  public $limit_per_entity = TRUE;
  public $response_entities = array();
  public $updated = REQUEST_TIME;
  public $created = REQUEST_TIME;

  /**
   * Override buildContent() to add the username to the output.
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    $content = array();

    return entity_get_controller($this->entityType)->buildContent($this, $view_mode, $langcode, $content);
  }

  /**
   * Specifies the default label, which is picked up by label() by default.
   */
  protected function defaultLabel() {
    return t('Response set: @name', array('@name' => $this->name));
  }

  /**
   * Specifies the default uri, which is picked up by uri() by default.
   */
  protected function defaultURI() {
    return array('path' => 'admin/structure/starter-response/' . $this->id);
  }

  /**
   * Save this Starter Response set.
   */
  public function save() {
    $this->updated = REQUEST_TIME;

    if (!empty($this->id)) {
      $unchanged = entity_load_unchanged('starter_response_set', $this->id);
      $removed = array_udiff($unchanged->response_entities, $this->response_entities, array('StarterResponseSet', 'starter_response_response_set_compare'));
      $added = array_udiff($this->response_entities, $unchanged->response_entities, array('StarterResponseSet', 'starter_response_response_set_compare'));

    }
    else {
      $added = $this->response_entities;
      $removed = array();
    }
    try {
      $fields = field_info_fields();
      if (!array_key_exists('starter_response_t_' . $this->start_entity, $fields)) {
        $field = array(
          'translatable' => '0',
          'settings' => array(
            'target_type' => $this->start_entity,
            'handler' => 'base',
            'handler_settings' => array(
              'target_bundles' => array(),
              'sort' => array(
                'type' => 'none',
              ),
              'behaviors' => array(
                'views-select-list' => array(
                  'status' => 0,
                ),
              ),
            ),
          ),
          'field_name' => 'starter_response_t_' . $this->start_entity,
          'type' => 'entityreference',
          'active' => '1',
          'locked' => '0',
          'cardinality' => '1',
        );
        field_create_field($field);
      }

      foreach ($added as $response_entity) {
        $instance = array(
          'label' => 'Starter Response creator',
          'widget' => array(
            'weight' => '6',
            'type' => 'options_select',
            'active' => 1,
            'settings' => array(),
          ),
          'settings' => array(
            'user_register_form' => FALSE,
          ),
          'display' => array(
            'default' => array(
              'label' => 'above',
              'type' => 'hidden',
              'settings' => array(
                'link' => FALSE,
              ),
              'weight' => 5,
            ),
            'teaser' => array(
              'type' => 'hidden',
              'label' => 'above',
              'settings' => array(),
              'weight' => 0,
            ),
          ),
          'required' => 0,
          'description' => '',
          'default_value' => NULL,
          'field_name' => 'starter_response_user',
          'entity_type' => $response_entity['entity'],
          'bundle' => $response_entity['bundle'],
        );
        field_create_instance($instance);

        $instance = array(
          'label' => 'Starter Response target',
          'widget' => array(
            'weight' => '6',
            'type' => 'options_select',
            'active' => 1,
            'settings' => array(),
          ),
          'settings' => array(
            'user_register_form' => FALSE,
          ),
          'display' => array(
            'default' => array(
              'label' => 'above',
              'type' => 'hidden',
              'settings' => array(
                'link' => FALSE,
              ),
              'weight' => 5,
            ),
            'teaser' => array(
              'type' => 'hidden',
              'label' => 'above',
              'settings' => array(),
              'weight' => 0,
            ),
          ),
          'required' => 0,
          'description' => '',
          'default_value' => NULL,
          'field_name' => 'starter_response_t_' . $this->start_entity,
          'entity_type' => $response_entity['entity'],
          'bundle' => $response_entity['bundle'],
        );
        field_create_instance($instance);
      }
      foreach ($removed as $response_entity) {
        $instance = field_read_instance($response_entity['entity'], 'starter_response_t_' . $this->start_entity, $response_entity['bundle']);
        field_delete_instance($instance, FALSE);
        $instance = field_read_instance($response_entity['entity'], 'starter_response_user', $response_entity['bundle']);
        field_delete_instance($instance, FALSE);
      }
    }
    catch (Exception $e) {
      // @TODO Better error handling/validation here....
    }
    parent::save();
  }

  public function delete() {
    foreach ($this->response_entities as $response_entity) {
      $instance = field_read_instance($response_entity['entity'], 'starter_response_t_' . $this->start_entity, $response_entity['bundle']);
      field_delete_instance($instance, FALSE);
      $instance = field_read_instance($response_entity['entity'], 'starter_response_user', $response_entity['bundle']);
      field_delete_instance($instance, FALSE);
    }
    parent::delete();
  }

  public function allowResponse($entity, $entity_type, $bundle) {
    $field = 'starter_response_t_' . $this->start_entity;
    $starter_id = $entity->{$field}[LANGUAGE_NONE][0]['target_id'];
    $result = $this->buildQuery($starterId, $entity_type, $bundle);

  }

  public function buildQuery() {
    $query = new EntityFieldQuery;
    $query->set

  }

  /**
   * Find all response sets that match the specified starter type and bundle.
   */
  public static function findByStarter($entity_type, $bundle = NULL) {
    $query = self::query()->where('start_entity', $entity_type);
    if ($bundle != NULL) {
      $query->where('start_bundle', $bundle);
    }
    return $query->results();
  }

  public static function findByResponse($entity_type, $bundle = NULL) {
    $query = self::query();
    $alias = $query->join('starter_response_set_responses', '%base.id = %alias.set_id');
    $query->joinCondition($alias, 'response_entity', $entity_type);
    if ($bundle != NULL) {
      $query->joinCondition($alias, 'response_bundle', $bundle);
    }
    return $query->results();
  }

  /**
   * Helper function for array_diff_uassoc
   */
  public static function starter_response_response_set_compare($a, $b) {
    if ($a['entity'] == $b['entity'] && $a['bundle'] == $b['bundle']) {
      return FALSE;
    }
    return TRUE;
  }


}
