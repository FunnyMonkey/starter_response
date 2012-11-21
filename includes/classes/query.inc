<?php

/**
 * Helper class for wrapping EntityFieldQuery
 */
class StarterResponseEntityFieldQuery extends EntityFieldQuery {
  public $joins = array();
  public $joinConditions = array();
  public $joinTables = array();

  protected $entity_type;

  /**
   * Public constructor allows setting an entity type for this query.
   *
   * @param $entity_type
   *   The type of entity to query for. This will be used to create a condition
   *   on entity_type.
   */
  public function __construct($entity_type) {
    $this->entityCondition('entity_type', $entity_type, '=');
    $this->entity_type = $entity_type;
  }

  /**
   * A helper wrapper around propertyCondition() that sets a default operator.
   *
   * @see EntityFieldQuery::propertyCondition().
   *
   * @return StarterResponseEntityFieldQuery
   */
  public function where($column, $value, $operator = '=') {
    return $this->propertyCondition($column, $value, $operator);
  }

  /**
   * A helper wrapper around propertyOrderBy().
   *
   * @see EntityFieldQuery::propertyOrderBy().
   *
   * @return StarterResponseEntityFieldQuery
   */
  public function order($column, $direction = 'ASC') {
    return $this->propertyOrderBy($column, $direction);
  }

  /**
   * A helper wrapper around range().
   *
   * @see EntityFieldQuery::range().
   *
   * @return StarterResponseEntityFieldQuery
   */
  public function limit($count) {
    return $this->range(0, $count);
  }

  /**
   * Reset all propertyConditions, order and range settings, and run the query
   * and return the results.
   *
   * @return array
   */
  public function all() {
    // Reset all properties (except entity type).
    $this->propertyConditions = array();
    $this->range = array();
    $this->order = array();
    $this->joins = array();
    $this->joinConditions = array();
    $this->count = FALSE;
    $this->fieldConditions = array();
    return $this->results();
  }

  /**
   * Set the query to a count query, and execute it.
   *
   * @return mixed
   *   Results from county query or FALSE.
   */
  public function count() {
    $this->count = TRUE;
    return $this->execute();
  }

  /**
   * Executes the query, and loads all matching entities.
   *
   * @return array
   *   An array of entity objects indexed by their ids. When no results are
   *   found, an empty array is returned.
   */
  public function results() {
    $results = parent::execute();
    if (isset($results[$this->entity_type])) {
      return entity_load($this->entity_type, array_keys($results[$this->entity_type]));
    }
    return array();
  }

  /**
   * Executes the query, loads and returns the first entity.
   *
   * @return Entity
   *   A single entity.
   */
  public function result() {
    $results = parent::execute();
    if (isset($results[$this->entity_type])) {
      return entity_load_single($this->entity_type, reset(array_keys($results[$this->entity_type])));
    }
    return FALSE;
  }

  public function join($table, $condition, $placeholders = array()) {
    $entity_type = $this->entityConditions['entity_type']['value'];
    $entity_info = entity_get_info($entity_type);
    $base_table = $entity_info['base table'];

    $alias = $this->getTableAlias($table);

    if (is_string($condition)) {
      $condition = str_replace('%alias', $alias, $condition);
      $condition = str_replace('%base', $base_table, $condition);
    }

    $this->joins[$alias] = array(
      'table' => $table,
      'condition' => $condition,
      'placeholders' => $placeholders,
    );
    return $alias;
  }

  public function joinCondition($alias, $field, $value, $operator = '=') {
    $this->joinConditions[] = array(
      'alias' => $alias,
      'field' => $field,
      'value' => $value,
      'operator' => $operator,
    );
    return $this;
  }

  /**
   * Get the alias for a table.
   */
  public function getTableAlias($table) {
    $existing_keys = array_keys($this->joins);
    $alias = $name = substr($table, 0, strpos($table, '_'));
    if (in_array($alias, $existing_keys)) {
      // Alias already exists
      $counter = 0;
      do {
        $alias = $name . '_' . $counter++;
      } while (in_array($alias, $existing_keys));
    }
    return $alias;
  }

  /**
   * Finishes the query.
   *
   * Adds tags, metaData, range and returns the requested list or count.
   *
   * @param SelectQuery $select_query
   *   A SelectQuery which has entity_type, entity_id, revision_id and bundle
   *   fields added.
   * @param $id_key
   *   Which field's values to use as the returned array keys.
   *
   * @return
   *   See EntityFieldQuery::execute().
   */
  function finishQuery($select_query, $id_key = 'entity_id') {
    foreach ($this->joins as $alias => $join) {
      $select_query->join($join['table'], $alias, $join['condition'], $join['placeholders']);
    }

    foreach ($this->joinConditions as $condition) {
      $select_query->condition($condition['alias'] . '.' . $condition['field'], $condition['value'], $condition['operator']);
    }

    return parent::finishQuery($select_query, $id_key);
  }
}
