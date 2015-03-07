<?php

/**
 * @file
 * Honeypot administration forms.
 */

/**
 * Honeypot administration page.
 */
function honeypot_admin_form($form, &$form_state) {
  $config = config('honeypot.settings');

  // Honeypot Configuration.
  $form['configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Honeypot Configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['configuration']['honeypot_protect_all_forms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Protect all forms with Honeypot'),
    '#description' => t('Enable Honeypot protection for ALL forms on this site (it is best to only enable Honeypot for the forms you need below).'),
    '#default_value' => $config->get('protect_all_forms'),
  );
  $form['configuration']['honeypot_protect_all_forms']['#description'] .= '<br />' . t('<strong>Page caching will be disabled on any page where a form is present if the Honeypot time limit is not set to 0.</strong>');
  $form['configuration']['honeypot_log'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log blocked form submissions'),
    '#description' => t('Log submissions that are blocked due to Honeypot protection.'),
    '#default_value' => $config->get('log'),
  );
  $form['configuration']['honeypot_element_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Honeypot element name'),
    '#description' => t("The name of the Honeypot form field. It's usually most effective to use a generic name like email, homepage, or name, but this should be changed if it interferes with fields that are already in your forms. Must not contain spaces or special characters."),
    '#default_value' => $config->get('element_name'),
    '#required' => TRUE,
    '#size' => 30,
  );
  $form['configuration']['honeypot_time_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Honeypot time limit'),
    '#description' => t('Minimum time required before form should be considered entered by a human instead of a bot. Set to 0 to disable.'),
    '#default_value' => $config->get('time_limit'),
    '#required' => TRUE,
    '#size' => 5,
    '#field_suffix' => t('seconds'),
  );
  $form['configuration']['honeypot_time_limit']['#description'] .= '<br />' . t('<strong>Page caching will be disabled if there is a form protected by time limit on the page.</strong>');

  // Honeypot Enabled forms.
  $form['enabled_forms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Honeypot Enabled Forms'),
    '#description' => t("Check the boxes next to individual forms on which you'd like Honeypot protection enabled."),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#states' => array(
      // Hide this fieldset when all forms are protected.
      'invisible' => array(
        'input[name="honeypot_protect_all_forms"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form_settings = $config->get('form_settings');

  // Generic forms.
  $form['enabled_forms']['general_forms'] = array('#markup' => '<h5>' . t('General Forms') . '</h5>');
  // User register form.
  $form['enabled_forms']['honeypot_form_user_register_form'] = array(
    '#type' => 'checkbox',
    '#title' => t('User Registration form'),
    '#default_value' => $form_settings['honeypot_form_user_register_form'],
  );
  // User password form.
  $form['enabled_forms']['honeypot_form_user_pass'] = array(
    '#type' => 'checkbox',
    '#title' => t('User Password Reset form'),
    '#default_value' => $form_settings['honeypot_form_user_pass'],
  );

  // If webform.module enabled, add webforms.
  if (module_exists('webform')) {
    $form['enabled_forms']['honeypot_form_webforms'] = array(
      '#type' => 'checkbox',
      '#title' => t('Webforms (all)'),
      '#default_value' => $form_settings['honeypot_form_webforms'],
    );
  }

  // If contact.module enabled, add contact forms.
  if (module_exists('contact')) {
    $form['enabled_forms']['contact_forms'] = array('#markup' => '<h5>' . t('Contact Forms') . '</h5>');
    // Sitewide contact form.
    $form['enabled_forms']['honeypot_form_contact_site_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Sitewide Contact form'),
      '#default_value' => $form_settings['honeypot_form_contact_site_form'],
    );
    // Sitewide personal form.
    $form['enabled_forms']['honeypot_form_contact_personal_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Personal Contact forms'),
      '#default_value' => $form_settings['honeypot_form_contact_personal_form'],
    );
  }

  // If profile.module enabled, add profile forms.
  if (module_exists('profile')) {
    $form['enabled_forms']['profile_forms'] = array('#value' => '<h5>' . t('Profile Forms') . '</h5>');
    $form['enabled_forms']['honeypot_form_user_profile_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Profile forms (all)'),
      '#default_value' => $form_settings['honeypot_form_user_profile_form'],
    );
  }

  // Get node types for node forms and node comment forms.
  $types = node_type_get_types();
  if (!empty($types)) {
    // Node forms.
    $form['enabled_forms']['node_forms'] = array('#markup' => '<h5>' . t('Node Forms') . '</h5>');
    foreach ($types as $type) {
      $id = 'honeypot_form_' . $type->type . '_node_form';
      $form['enabled_forms'][$id] = array(
        '#type' => 'checkbox',
        '#title' => t('@name node form', array('@name' => $type->name)),
        '#default_value' => $form_settings[$id],
      );
    }

    // Comment forms.
    if (module_exists('comment')) {
      $form['enabled_forms']['comment_forms'] = array('#markup' => '<h5>' . t('Comment Forms') . '</h5>');
      foreach ($types as $type) {
        $id = 'honeypot_form_comment_node_' . $type->type . '_form';
        $form['enabled_forms'][$id] = array(
          '#type' => 'checkbox',
          '#title' => t('@name comment form', array('@name' => $type->name)),
          '#default_value' => $form_settings[$id],
        );
      }
    }
  }

  // Submit button.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validate the admin form.
 */
function honeypot_admin_form_validate($form, &$form_state) {
  // Make sure the time limit is a positive integer or 0.
  $time_limit = $form_state['values']['honeypot_time_limit'];
  if ((is_numeric($time_limit) && $time_limit > 0) || $time_limit === '0') {
    if (ctype_digit($time_limit)) {
      // Good to go.
    }
    else {
      form_set_error('honeypot_time_limit', t("The time limit must be a positive integer or 0."));
    }
  }
  else {
    form_set_error('honeypot_time_limit', t("The time limit must be a positive integer or 0."));
  }

  // Make sure Honeypot element name only contains A-Z, 0-9.
  if (!preg_match("/^[-_a-zA-Z0-9]+$/", $form_state['values']['honeypot_element_name'])) {
    form_set_error('honeypot_element_name', t("The element name cannot contain spaces or other special characters."));
  }
}

/**
 * Honeypot admin form submit callback.
 */
function honeypot_admin_form_submit($form, &$form_state) {
  // Create CSS file for honeypot.
  honeypot_create_css($form_state['values']['honeypot_element_name']);

  $config = config('honeypot.settings');

  // Store individual form settings in an array
  $form_settings = array();

  foreach($form_state['values'] as $k => $v) {
    if("honeypot_form_" == substr($k,0,14)) {
      $form_settings[$k] = $v;
    }
  }

  // Save all the Honeypot configuration items from $form_state.
  $config->set('protect_all_forms', $form_state['values']['honeypot_protect_all_forms']);
  $config->set('log', $form_state['values']['honeypot_log']);
  $config->set('element_name', $form_state['values']['honeypot_element_name']);
  $config->set('time_limit', $form_state['values']['honeypot_time_limit']);
  $config->set('form_settings', $form_settings);

  $config->save();

  // Clear the Honeypot form cache on submit.
  cache_clear_all('honeypot_protected_forms', 'cache');

  backdrop_set_message(t('The honeypot settings have been saved.'), 'status');
}
