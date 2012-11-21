<?php

class StarterResponseSetEntityController extends EntityAPIController implements EntityAPIControllerInterface {

  /**
   * Implements EntityAPIControllerInterface.
   *
   * @param $transaction
   *   Optionally a DatabaseTransaction object to use. Allows overrides to pass
   *   in their transaction object.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $transaction = isset($transaction) ? $transaction : db_transaction();

    try {
      if (!empty($entity->{$this->idKey}) && empty($entity->is_new)) {
        // Don't delete unless this is a new entity
        db_delete('starter_response_set_responses')
          ->condition('set_id', $entity->id)
          ->execute();
      }

      // Save the entity in order to get our lid
      parent::save($entity, $transaction);

      // Save the allowed roles
      if (isset($entity->response_entities) && !empty($entity->response_entities)) {
        $query = db_insert('starter_response_set_responses')
          ->fields(array('set_id', 'response_entity', 'response_bundle'));
        foreach ($entity->response_entities as $entity_type => $bundles) {
          if (!empty($bundles)) {
            foreach ($bundles as $bundle) {
              if (!empty($bundle)) {
                $query->values(array('set_id' => $entity->id, 'response_entity' => $entity_type, 'response_bundle' => $bundle));
              }
            }
          }
        }
        $query->execute();
      }
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception($this->entityType, $e);
      throw $e;
    }
  }

  /**
   * Overridden.
   * @see DrupalDefaultEntityController#load($ids, $conditions)
   *
   * In contrast to the parent implementation we factor out query execution, so
   * fetching can be further customized easily.
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);

    $ids = array_keys($entities);

    if (!empty($ids)) {
      $rows = db_select('starter_response_set_responses', 'srsr')
        ->fields('srsr')
        ->condition('set_id', $ids, 'IN')
        ->execute();

      foreach ($rows as $row) {
        if (isset($entities[$row->set_id]->response_entities[$row->response_entity])) {
          $entities[$row->set_id]->response_entities[$row->response_entity][] = $row->response_bundle;
        }
        else {
          $entities[$row->set_id]->response_entities[$row->response_entity] = array($row->response_bundle);
        }
      }
    }


    return $entities;
  }

  /**
   * Implements EntityAPIControllerInterface.
   *
   * @param $transaction
   *   Optionally a DatabaseTransaction object to use. Allows overrides to pass
   *   in their transaction object.
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $entities = $ids ? $this->load($ids) : FALSE;
    if (!$entities) {
      // Do nothing, in case invalid or no ids have been passed.
      return;
    }
    $transaction = isset($transaction) ? $transaction : db_transaction();

    try {
      parent::delete($ids, $transaction);

      db_delete('starter_response_set_responses')
        ->condition('set_id', $ids, 'IN')
        ->execute();

    }
    catch (Exception $e) {
      if (isset($transaction)) {
        $transaction->rollback();
      }
      watchdog_exception($this->entityType, $e);
      throw $e;
    }
  }
}
