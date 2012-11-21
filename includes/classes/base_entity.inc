<?php

/**
 * Base class for all Starter Response entities.
 */
abstract class StarterResponseBaseEntity extends Entity {
  public $starter_response_entity_type;

  /**
   * Creates a new entity.
   *
   * @param $values
   *   An array of values to set, keyed by property name. If the entity type has
   *   bundles the bundle key has to be specified.
   */
  public function __construct(array $values = array()) {
    $type = $this->get_entity_type();
    $this->starter_response_entity_type = $type;
    parent::__construct($values, $type);
  }


  /**
   * Generic entry point to load or search for entties.
   *
   * @static
   *
   * @param mixed $id Optional
   *   An ID or array of IDs, or can be left blank to return an EntityFieldQuery
   *   with the entity_type condition preset.
   *
   * @return mixed
   *   A single item, an array of items, or a pre-conditioned EntityFieldQuery
   *   depending on the value of $id.
   */
  public static function find($id = NULL) {
    if ($id === NULL) {
      // Assume we are querying for items
      return self::query();
    }
    elseif (is_array($id)) {
      // We are looking up multiple IDs
      return self::load($id);
    }
    else {
      // We are looking up a single item
      return entity_load_single(self::get_entity_type_static(), $id);
    }
  }

  /**
   * @static
   * Static wrapper for entity_load(). Presets the entity type.
   *
   * @param array $ids
   *   An array of entity IDs, or FALSE to load all entities.
   *
   * @return mixed
   *   An array of entity objects indexed by their ids. When no results are
   *   found, an empty array is returned.
   */
  public static function load(array $ids = array()) {
    return entity_load(self::get_entity_type_static(), $ids);
  }

  /**
   * @static
   * Return an EntityFieldQuery with preconditions for entity type.
   *
   * @return StarterResponseEntityFieldQuery
   */
  public static function query() {
    $query = new StarterResponseEntityFieldQuery(self::get_entity_type_static());
    return $query;
  }

  /**
   * Return an EntityMetadataWrapper for this entity.
   *
   * @return EntityMetadataWrapper
   */
  public function wrapper() {
    return entity_metadata_wrapper($this->starter_response_entity_type, $this);
  }

  /**
   * Set the values of this
   */
  public function values(array $values = array()) {
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }
  }

  protected function get_entity_type() {
    $class = get_class($this);
    $types = entity_get_info();
    foreach ($types as $entity_type => $info) {
      if (isset($info['entity class']) && $class == $info['entity class']) {
        return $entity_type;
      }
    }
  }

  protected static function get_entity_type_static() {
    $class = get_called_class();
    $types = entity_get_info();
    foreach ($types as $entity_type => $info) {
      if (isset($info['entity class']) && $class == $info['entity class']) {
        return $entity_type;
      }
    }
  }
}
