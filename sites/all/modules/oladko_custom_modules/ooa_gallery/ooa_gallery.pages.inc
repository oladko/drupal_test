<?php

/**
 * Task view callback.
 */
function ooa_gallery_view($gallery) {
  drupal_set_title(entity_label('ooa_gallery', $gallery));
  return entity_view('ooa_gallery', array(entity_id('ooa_gallery', $gallery) => $gallery), 'full');
}
