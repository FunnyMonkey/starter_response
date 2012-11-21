<?php

/**
 * Default class for Lab entities.
 */
class StarterResponseSet extends StarterResponseBaseEntity {
  public $id;
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
    parent::save();
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
}
