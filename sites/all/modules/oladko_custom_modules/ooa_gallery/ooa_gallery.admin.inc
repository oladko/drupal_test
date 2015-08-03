<?php

/**
 * Generates the gallery type editing form.
 */
function ooa_gallery_type_form($form, &$form_state, $gallery_type, $op = 'edit') {

  if ($op == 'clone') {
    $gallery_type->label .= ' (cloned)';
    $gallery_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $gallery_type->label,
    '#description' => t('The human-readable name of this gallery type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($gallery_type->type) ? $gallery_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $gallery_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'ooa_gallery_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this gallery type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($gallery_type->description) ? $gallery_type->description : '',
    '#description' => t('Description about the gallery type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save gallery type'),
    '#weight' => 40,
  );

  if (!$gallery_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete gallery type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('ooa_gallery_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Submit handler for creating/editing gallery_type.
 */
function ooa_gallery_type_form_submit(&$form, &$form_state) {
  $gallery_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  ooa_gallery_type_save($gallery_type);

  // Redirect user back to list of task types.
  $form_state['redirect'] = 'admin/structure/gallery-types';
}

function ooa_gallery_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/gallery-types/' . $form_state['ooa_gallery_type']->type . '/delete';
}

/**
 * Gallery type delete form.
 */
function ooa_gallery_type_form_delete_confirm($form, &$form_state, $gallery_type) {
  $form_state['gallery_type'] = $gallery_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['gallery_type_id'] = array('#type' => 'value', '#value' => entity_id('ooa_gallery_type', $gallery_type));
  return confirm_form($form,
    t('Are you sure you want to delete gallery type %title?', array('%title' => entity_label('gallery_type', $gallery_type))),
    'gallery/' . entity_id('ooa_gallery_type', $gallery_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Gallery type delete form submit handler.
 */
function ooa_gallery_type_form_delete_confirm_submit($form, &$form_state) {
  $gallery_type = $form_state['gallery_type'];
  ooa_gallery_type_delete($gallery_type);

  watchdog('ooa_gallery_type', '@type: deleted %title.', array('@type' => $gallery_type->type, '%title' => $gallery_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $gallery_type->type, '%title' => $gallery_type->label)));

  $form_state['redirect'] = 'admin/structure/gallery-types';
}

/**
 * Page to select gallery Type to add new gallery.
 */
function ooa_gallery_admin_add_page() {
  $items = array();
  foreach (ooa_gallery_types() as $gallery_type_key => $gallery_type) {
    $items[] = l(entity_label('ooa_gallery_type', $gallery_type), 'gallery/add/' . $gallery_type_key);
  }
  return array('list' => array('#theme' => 'item_list', '#items' => $items, '#title' => t('Select type of gallery to create.')));
}

/**
 * Add new gallery page callback.
 */
function ooa_gallery_add($type) {
  $gallery_type = ooa_gallery_types($type);

  $gallery = entity_create('ooa_gallery', array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('gallery_type', $gallery_type))));

  $output = drupal_get_form('ooa_gallery_form', $gallery);

  return $output;
}

/**
 * Gallery Form.
 */
function ooa_gallery_form($form, &$form_state, $gallery) {
  $form_state['gallery'] = $gallery;

  $form['title'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Name'),
    '#default_value' => $gallery->title,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $gallery->description,
  );

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $gallery->uid,
  );

  field_attach_form('ooa_gallery', $gallery, $form, $form_state);

  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save gallery'),
    '#submit' => $submit + array('ooa_gallery_form_submit'),
  );

  // Show Delete button if we edit gallery.
  $gallery_id = entity_id('ooa_gallery', $gallery);
  if (!empty($gallery_id) && ooa_gallery_access('edit', $gallery)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('ooa_gallery_form_submit_delete'),
    );
  }

  $form['#validate'][] = 'ooa_gallery_form_validate';

  return $form;
}

function ooa_gallery_form_validate($form, &$form_state) {

}

/**
 * Gallery submit handler.
 */
function ooa_gallery_form_submit($form, &$form_state) {
  $gallery = $form_state['gallery'];

  entity_form_submit_build_entity('ooa_gallery', $gallery, $form, $form_state);

  ooa_gallery_save($gallery);

  $gallery_uri = entity_uri('ooa_gallery', $gallery);

  $form_state['redirect'] = $gallery_uri['path'];

  drupal_set_message(t('Gallery %title saved.', array('%title' => entity_label('ooa_gallery', $gallery))));
}

function ooa_gallery_form_submit_delete($form, &$form_state) {
  $gallery = $form_state['gallery'];
  $gallery_uri = entity_uri('ooa_gallery', $gallery);
  $form_state['redirect'] = $gallery_uri['path'] . '/delete';
}

/**
 * Delete confirmation form.
 */
function ooa_gallery_delete_form($form, &$form_state, $gallery) {
  $form_state['gallery'] = $gallery;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['gallery_type_id'] = array('#type' => 'value', '#value' => entity_id('ooa_gallery', $gallery));
  $gallery_uri = entity_uri('ooa_gallery', $gallery);
  return confirm_form($form,
    t('Are you sure you want to delete gallery %title?', array('%title' => entity_label('ooa_gallery', $gallery))),
    $gallery_uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function ooa_gallery_delete_form_submit($form, &$form_state) {
  $gallery = $form_state['gallery'];
  ooa_gallery_delete($gallery);

  drupal_set_message(t('Gallery %title deleted.', array('%title' => entity_label('ooa_gallery', $gallery))));

  $form_state['redirect'] = '<front>';
}
