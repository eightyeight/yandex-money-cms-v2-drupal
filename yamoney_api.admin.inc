<?php
/**
 * @return array
 */
function yamoney_api_admin_settings()
{
    global $base_url;
    $path         = drupal_get_path('module', 'yamoney_api').'/'.'yamoney_api.info';
    $info         = drupal_parse_info_file($path);
    $shopPassword = variable_get('yamoney_api_secret', '');
    $prefix       = substr($shopPassword, 0, 4);

    if ($prefix == "test") {
        drupal_set_message(
            'Вы включили тестовый режим приема платежей. Проверьте, как проходит оплата. <a href="https://kassa.yandex.ru/">Подробнее</a>',
            'warning'
        );
    }

    remove_duplicate_messages('warning');

    $form                                      = array();
    $form['yamoney_api_all']                   = array(
        '#type'        => 'fieldset',
        '#description' => t(
            'Работая с модулем, вы автоматически соглашаетесь с <a href="https://money.yandex.ru/doc.xml?id=527132" target="_blank">условиями его использования</a>.'
        ),
        '#weight'      => 10,
    );
    $form['yamoney_api_all']['lbl_version']    = array(
        '#markup' => t('Version: ').$info['version'],
    );
    $form['yamoney_api_all']['yamoney_api_ip'] = array(
        '#type'          => 'hidden',
        '#title'         => t('Allowed IPs for callbacks'),
        '#default_value' => variable_get('yamoney_api_ip', '0.0.0.0'),
        '#description'   => t(
            'The list of IP addresses which has access to payment callbacks. One per line.<br/>0.0.0.0 means allow from all.'
        ),
    );

    $form['vertical_tabs']                        = array(
        '#type'   => 'vertical_tabs',
        '#weight' => 20,
    );
    $form['yamoney_api_kassa_tab']                = array(
        '#type'   => 'fieldset',
        '#title'  => t('Yandex.Kassa'),
        '#group'  => 'vertical_tabs',
        '#weight' => 10,
    );
    $form['yamoney_api_kassa_tab']['lbl_setting'] = array(
        '#markup' => t('<h3>Shop setting</h3>'),
    );
    $form['yamoney_api_kassa_tab']['intro']       = array(
        '#markup' => t(
            'Для работы с модулем нужно подключить магазин к <a target="_blank" href="https://kassa.yandex.ru/">Яндекс.Кассе</a>.'
        ),
    );

    $form['yamoney_api_kassa_tab']['yamoney_api_shop'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Включить приём платежей через Яндекс.Кассу'),
        '#attributes'    => array('id' => 'kassa-checkbox'),
        '#default_value' => variable_get('yamoney_api_shop', 0),
    );


    $form['yamoney_api_kassa_tab']['yamoney_api_shop_id'] = array(
        '#type'          => 'textfield',
        '#title'         => t('shopId'),
        '#description'   => t('Скопируйте shopId из личного кабинета Яндекс.Кассы'),
        '#default_value' => variable_get('yamoney_api_shop_id', ''),
        '#size'          => 4,
    );
    $form['yamoney_api_kassa_tab']['yamoney_api_secret']  = array(
        '#type'          => 'textfield',
        '#title'         => t('Секретный ключ'),
        '#description'   => t(
            'Выпустите и активируйте секретный ключ в личном кабинете Яндекс.Кассы. Потом скопируйте его сюда.'
        ),
        '#default_value' => variable_get('yamoney_api_secret', ''),
        '#size'          => 17,
    );

    $paymode_states                                       = array(
        'invisible' => array(
            ':input[name="yamoney_api_paymode"]' => array('value' => 'kassa'),
        ),
    );
    $form['yamoney_api_kassa_tab']['yamoney_api_paymode'] = array(
        '#type'          => 'select',
        '#title'         => t('Payments mode'),
        '#options'       => array(
            'kassa' => t('Kassa mode'),
            'shop'  => t('Shop mode'),
        ),
        '#default_value' => variable_get('yamoney_api_paymode', 'kassa'),
    );
    /*$form['yamoney_api_kassa_tab']['yamoney_api_paylogo'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use logo of payment button'),
        '#default_value' => variable_get('yamoney_api_paylogo', 1),
        '#states' => array(
            'invisible' => array(
                ':input[name="yamoney_api_paymode"]' => array('value' => 'shop'),
            ),
        ),
    );*/
    $form['yamoney_api_kassa_tab']['yamoney_api_payment_method'] = array(
        '#type'          => 'checkboxes',
        '#title'         => t('Enabled payment methods'),
        '#options'       => yamoney_api_get_payment_methods(),
        '#default_value' => variable_get('yamoney_api_payment_method', array_keys(yamoney_api_get_payment_methods())),
        '#states'        => $paymode_states,
    );

    $form['yamoney_api_kassa_tab']['yamoney_api_default_payment_method'] = array(
        '#type'          => 'radios',
        '#title'         => t('Default payment method'),
        '#options'       => yamoney_api_get_payment_methods(),
        '#default_value' => variable_get('yamoney_api_default_payment_method', YAMONEY_API_DEFAULT_PAYMENT_METHOD),
        '#states'        => $paymode_states,
    );


    /*********** tax edit****************/

    if (module_exists('commerce_tax') && module_exists('commerce_tax_ui')) {
        $form['yamoney_api_kassa_tab']['yamoney_api_send_check'] = array(
            '#type'          => 'radios',
            '#title'         => t('Отправлять в Яндекс.Кассу данные для чеков (54-ФЗ)'),
            '#options'       => array(
                '1' => t('Включить'),
                '0' => t('Выключить'),
            ),
            '#default_value' => variable_get('yamoney_api_send_check', '0'),
        );

        $taxes = commerce_tax_ui_commerce_tax_rate_info();

        if ($taxes && variable_get('yamoney_api_send_check')) {
            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax'] = array(
                '#markup' => '<label>Сопоставьте ставки</label>',
            );

            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax_start'] = array(
                '#markup' => '<div>',
            );
            $form['yamoney_api_kassa_tab']['lbl_shop_tax']          = array(
                '#markup' => t('<div style="float: left;width: 200px;">Ставка в вашем магазине.</div>'),
            );
            $form['yamoney_api_kassa_tab']['lbl_kassa_tax']         = array(
                '#markup' => t('<div>Ставка для чека в налоговую.</div>'),
            );
            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax_end']   = array(
                '#markup' => '</div>',
            );

            foreach ($taxes as $tax) {
                $form['yamoney_api_kassa_tab']['lbl_yamoney_api_kassa_tax_'.$tax['name']] = array(
                    '#markup' => '<div> <label style="width: 200px;float: left;padding-top: 10px;">'.$tax['title'].'</label>',
                );
                $form['yamoney_api_kassa_tab']['yamoney_api_kassa_tax_'.$tax['name']]     = array(
                    '#type'          => 'select',
                    '#title_display' => false,
                    '#options'       => array(
                        1 => t('Без НДС'),
                        2 => t('0%'),
                        3 => t('10%'),
                        4 => t('18%'),
                        5 => t('Расчётная ставка 10/110'),
                        6 => t('Расчётная ставка 18/118'),
                    ),
                    '#default_value' => variable_get('yamoney_api_kassa_tax_'.$tax['name'], 1),
                );

                $form['yamoney_api_kassa_tab']['lbl_yamoney_api_kassa_tax_'.$tax['name'].'_end'] = array(
                    '#markup' => '</div>',
                );
            }
        }
    }

    /*********** tax edit****************/

    if (module_exists('uc_taxes')) {
        $form['yamoney_api_kassa_tab']['yamoney_api_send_check'] = array(
            '#type'          => 'radios',
            '#title'         => t('Отправлять в Яндекс.Кассу данные для чеков (54-ФЗ)'),
            '#options'       => array(
                '1' => t('Включить'),
                '0' => t('Выключить'),
            ),
            '#default_value' => variable_get('yamoney_api_send_check', '0'),
        );

        $taxes = uc_taxes_rate_load();

        if ($taxes && variable_get('yamoney_api_send_check')) {
            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax'] = array(
                '#markup' => '<label>Сопоставьте ставки</label>',
            );

            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax_start'] = array(
                '#markup' => '<div>',
            );
            $form['yamoney_api_kassa_tab']['lbl_shop_tax']          = array(
                '#markup' => t('<div style="float: left;width: 200px;">Ставка в вашем магазине.</div>'),
            );
            $form['yamoney_api_kassa_tab']['lbl_kassa_tax']         = array(
                '#markup' => t('<div>Ставка для чека в налоговую.</div>'),
            );
            $form['yamoney_api_kassa_tab']['lbl_wrapper_tax_end']   = array(
                '#markup' => '</div>',
            );

            foreach ($taxes as $tax) {

                $form['yamoney_api_kassa_tab']['lbl_yamoney_api_kassa_tax_'.$tax->id] = array(
                    '#markup' => '<div> <label style="width: 200px;float: left;padding-top: 10px;">'.$tax->name.'</label>',
                );

                $form['yamoney_api_kassa_tab']['yamoney_api_kassa_tax_'.$tax->id] = array(
                    '#type'          => 'select',
                    '#label'         => false,
                    '#options'       => array(
                        1 => t('Без НДС'),
                        2 => t('0%'),
                        3 => t('10%'),
                        4 => t('18%'),
                        5 => t('Расчётная ставка 10/110'),
                        6 => t('Расчётная ставка 18/118'),
                    ),
                    '#default_value' => variable_get('yamoney_api_kassa_tax_'.$tax->id, 1),
                    '#description'   => '',
                );

                $form['yamoney_api_kassa_tab']['lbl_yamoney_api_kassa_tax_'.$tax->id.'_end'] = array(
                    '#markup' => '</div>',
                );
            }
        }
    }

    /*********** tax edit****************/
    if (module_exists('commerce')) {
        $form['yamoney_api_kassa_tab']['notifyUrl'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Url для уведомлений'),
            '#attributes'    => array('disabled' => 'disabled'),
            '#description'   => t('Этот адрес понадобится, только если его попросят специалисты Яндекс.Кассы'),
            '#default_value' => url('yamoney_api/commerce/notify', array('absolute' => true)),
        );
    }

    if (module_exists('uc_payment')) {
        $form['yamoney_api_kassa_tab']['notifyUrl'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Url для уведомлений'),
            '#attributes'    => array('disabled' => 'disabled'),
            '#description'   => t('Этот адрес понадобится, только если его попросят специалисты Яндекс.Кассы'),
            '#default_value' => url('yamoney_api/ubercart/notify', array('absolute' => true)),
        );
    }

    $form['yamoney_api_kassa_tab']['yamoney_api_debug'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Запись отладочной информации'),
        '#description'   => t('Настройку нужно будет поменять, только если попросят специалисты Яндекс.Денег'),
        '#attributes'    => array('id' => 'kassa-checkbox'),
        '#default_value' => variable_get('yamoney_api_debug', 0),
    );

    if (variable_get('yamoney_api_debug')) {
        $form['yamoney_api_kassa_tab']['my_button']['#type']                     = 'button';
        $form['yamoney_api_kassa_tab']['my_button']['#value']                    = t('Скачать лог');
        $form['yamoney_api_kassa_tab']['my_button']['#submit']                   = array('yamoney_api_log_handler');
        $form['yamoney_api_kassa_tab']['my_button']['#executes_submit_callback'] = true;
    }

    /* ---- */
    $form['yamoney_api_p2p_tab']                            = array(
        '#type'   => 'fieldset',
        '#title'  => t('Yandex.Money'),
        '#group'  => 'vertical_tabs',
        '#weight' => 30,
    );
    $form['yamoney_api_p2p_tab']['intro']                   = array(
        '#markup' => t(
            'Для работы с модулем нужно <a href="https://money.yandex.ru/new" target="_blank">открыть кошелек</a> на Яндексе и <a href="https://sp-money.yandex.ru/myservices/online.xml" target="_blank">зарегистрировать приложение</a> на сайте Яндекс.Денег'
        ),
    );
    $form['yamoney_api_p2p_tab']['yamoney_api_p2p']         = array(
        '#type'          => 'checkbox',
        '#attributes'    => array('id' => 'p2p-checkbox'),
        '#title'         => t('Включить прием платежей в кошелек на Яндексе'),
        '#default_value' => variable_get('yamoney_api_p2p', 0),
    );
    $form['yamoney_api_p2p_tab']['lbl_redirectUrl']         = array(
        '#type'          => 'textfield',
        '#title'         => t('RedirectURL'),
        '#attributes'    => array('disabled' => 'disabled'),
        '#default_value' => url('yamoney/check_wallet', array('absolute' => true)),
        '#description'   => t(
            "Скопируйте эту ссылку в верхнее поле на <a href='https://sp-money.yandex.ru/myservices/online.xml' target='_blank'>странице HTTP-уведомлений</a>"
        ),
    );
    $form['yamoney_api_p2p_tab']['lbl_setting']             = array(
        '#markup' => t('<h3>Настройки приема платежей</h3>'),
    );
    $form['yamoney_api_p2p_tab']['yamoney_api_receiver']    = array(
        '#type'          => 'textfield',
        '#title'         => t('Purse number'),
        '#description'   => t('Your Yandex.Money purse number.'),
        '#default_value' => variable_get('yamoney_api_receiver', ''),
        '#size'          => 14,
        //'#states' => $p2p_states,
    );
    $form['yamoney_api_p2p_tab']['yamoney_secret']          = array(
        '#type'          => 'textfield',
        '#title'         => t('Секретное слово'),
        '#description'   => t('Секретное слово'),
        '#default_value' => variable_get('yamoney_secret', ''),
        '#size'          => 17,
    );
    $form['yamoney_api_p2p_tab']['yamoney_api_formcomment'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Pay comment'),
        '#description'   => t('Это название увидит пользователь'),
        '#default_value' => variable_get('yamoney_api_formcomment', $base_url),
        //'#states' => $p2p_states,
    );

    // footer settings
    $form['yamoney_api_texts']                             = array(
        '#type'   => 'fieldset',
        '#title'  => t('Text for success and fail payment pages'),
        '#weight' => 40,
    );
    $success                                               = variable_get(
        'yamoney_api_success_text',
        array('value' => '', 'format' => '')
    );
    $form['yamoney_api_texts']['yamoney_api_success_text'] = array(
        '#type'          => 'text_format',
        '#title'         => t('Text for success page'),
        '#default_value' => $success['value'] ? $success['value'] : '',
        '#format'        => $success['format'] ? $success['format'] : '',
    );
    $fail                                                  = variable_get(
        'yamoney_api_fail_text',
        array('value' => '', 'format' => '')
    );
    $form['yamoney_api_texts']['yamoney_api_fail_text']    = array(
        '#type'          => 'text_format',
        '#title'         => t('Text for fail page'),
        '#default_value' => $fail['value'] ? $fail['value'] : '',
        '#format'        => $fail['format'] ? $fail['format'] : '',
    );
    $form['#submit'][]                                     = 'send_statistics';

    /** Fast Pay */

    $form['yamoney_api_billing_tab'] = array(
        '#type'   => 'fieldset',
        '#title'  => t('Yandex.Billing'),
        '#group'  => 'vertical_tabs',
        '#weight' => 50,
    );

    $form['yamoney_api_billing_tab']['intro'] = array(
        '#markup' => t(
            'This is a payment form for your site. It allows for accepting payments to your company account from cards and Yandex.Money e-wallets without a contract. To set it up, you need to provide the Yandex.Billing identifier: we will send it via email after you <a href=\'https://money.yandex.ru/fastpay\'>create a form in construction kit.</a>'
        ),
    );

    $form['yamoney_api_billing_tab']['yamoney_api_billing_enable'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Activate payments via Yandex.Billing'),
        '#attributes'    => array('id' => 'billing-checkbox'),
        '#default_value' => variable_get('yamoney_api_billing_enable'),
    );

    $form['yamoney_api_billing_tab']['yamoney_api_billing_id'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Yandex.Billing\'s identifier'),
        '#default_value' => variable_get('yamoney_api_billing_id', ''),
        '#size'          => 30,
    );

    $form['yamoney_api_billing_tab']['yamoney_api_billing_narrative'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Payment purpose'),
        '#description'   => t(
            'Payment purpose is added to the payment order: specify whatever will help identify the order paid via Yandex.Billing'
        ),
        '#default_value' => variable_get(
            'yamoney_api_billing_narrative',
            t('Order No. %order_id% Payment via Yandex.Billing')
        ),
        '#size'          => 30,
    );

    return system_settings_form($form);
}

/**
 * @param array $form
 * @param array $form_state
 */
function yamoney_api_admin_settings_validate($form, &$form_state)
{
    $checkboxs = $form_state['values']['yamoney_api_payment_method'];

    if ($form_state['values']['yamoney_api_shop'] == $form_state['values']['yamoney_api_p2p'] && $form_state['values']['yamoney_api_shop']) {
        $form_state['values']['yamoney_api_shop'] = 1;
        $form_state['values']['yamoney_api_p2p']  = 0;
        drupal_set_message(
            'Невозможно одновременно включить оплату через Яндекс.Деньги и Яндекс.Кассу',
            'warning'
        );
        form_set_error('yamoney_api_shop');
        form_set_error('yamoney_api_p2p');
    }

    if ($form_state['values']['yamoney_api_shop'] == $form_state['values']['yamoney_api_billing_enable'] && $form_state['values']['yamoney_api_shop']) {
        $form_state['values']['yamoney_api_shop']           = 1;
        $form_state['values']['yamoney_api_billing_enable'] = 0;
        drupal_set_message(
            'Невозможно одновременно включить оплату через Яндекс.Платежку и Яндекс.Кассу',
            'warning'
        );
        form_set_error('yamoney_api_shop');
        form_set_error('yamoney_api_billing_enable');
    }

    if ($form_state['values']['yamoney_api_p2p'] == $form_state['values']['yamoney_api_billing_enable'] && $form_state['values']['yamoney_api_p2p']) {
        $form_state['values']['yamoney_api_p2p']            = 1;
        $form_state['values']['yamoney_api_billing_enable'] = 0;
        drupal_set_message(
            'Невозможно одновременно включить оплату через Яндекс.Деньги и Яндекс.Кассу',
            'warning'
        );
        form_set_error('yamoney_api_p2p');
        form_set_error('yamoney_api_billing_enable');
    }


    if ($form_state['values']['yamoney_api_shop'] == 1) {
        if ($form_state['values']['yamoney_api_paymode'] == 'shop' && !count(array_filter($checkboxs))) {
            form_set_error(
                'yamoney_api_payment_method',
                'Не выбрано ни одного способа оплаты'
            );
        }

        if (!preg_match('/^\d+$/i', $form_state['values']['yamoney_api_shop_id'])) {
            form_set_error(
                'yamoney_api_shop_id',
                ' Такого shopId нет. Пожалуйста, скопируйте параметр в 
                <a href="https://money.yandex.ru/joinups">личном кабинете Яндекс.Кассы</a>
                (наверху любой страницы)'
            );
        }

        if (!preg_match('/^test_.*|live_.*$/i', $form_state['values']['yamoney_api_secret'])) {
            form_set_error(
                'yamoney_api_secret',
                ' Такого секретного ключа нет. Если вы уверены, что скопировали ключ правильно, значит, он по какой-то причине не работает.
                  Выпустите и активируйте ключ заново — 
                  <a href="https://money.yandex.ru/joinups">в личном кабинете Яндекс.Кассы</a>'
            );
        }
    }

    if ($form_state['values']['yamoney_api_p2p'] == 1) {
        if (empty($form_state['values']['yamoney_api_receiver'])) {
            form_set_error(
                'yamoney_api_receiver',
                'Заполните поле Номер кошелька'
            );
        }

        if (empty($form_state['values']['yamoney_secret'])) {
            form_set_error(
                'yamoney_secret',
                'Заполните поле Секретное слово'
            );
        }

        if (empty($form_state['values']['yamoney_api_formcomment'])) {
            form_set_error(
                'yamoney_api_formcomment',
                'Заполните поле Комментарий платежа'
            );
        }

    }

    if ($form_state['values']['yamoney_api_billing_enable'] == 1) {
        if (empty($form_state['values']['yamoney_api_billing_id'])) {
            form_set_error(
                'yamoney_api_billing_id',
                'Заполните поле Идентификатор платежки'
            );
        }

        if (empty($form_state['values']['yamoney_api_billing_narrative'])) {
            form_set_error(
                'yamoney_api_billing_narrative',
                'Заполните поле Назначение платежа'
            );
        }
    }

    if ($form_state['values']['yamoney_api_ip']) {
        $ips = explode("\n", $form_state['values']['yamoney_api_ip']);
        foreach ($ips as $ip) {
            $ip = trim($ip);
            if (empty($ip)) {
                continue;
            }
            if (!filter_var($ip, FILTER_VALIDATE_IP)) {
                form_set_error(
                    'yamoney_api_ip',
                    t(
                        'IP is not correct: %ip.<br/>Please insert correct server IP or insert 0.0.0.0 to default!',
                        array(
                            '%ip' => $ip,
                        )
                    )
                );
            }
        }
    }
}

function yamoney_api_form_alter(&$form, &$form_state, $form_id)
{
    YandexMoneyLogger::error($form_id);
}

function yamoney_api_admin_settings_alter(&$form, &$form_state)
{

}


function yamoney_api_log_handler()
{
    $path      = variable_get('file_public_path', conf_path().'/files');
    $filePath  = $path.'/ym-checkout-debug.log';
    $file      = fopen($filePath, 'r');
    $file_size = filesize($filePath);

    header("Expires: 0");
    header("Cache-Control: no-cache, no-store, must-revalidate");
    header('Cache-Control: pre-check=0, post-check=0, max-age=0', false);
    header("Pragma: no-cache");
    header("Content-Disposition:attachment; filename=debug.log");
    header("Content-Type: application/force-download");
    while (!feof($file)) {
        $buffer = fread($file, 2048);
        echo $buffer;
        flush();
    }
    fclose($file);
    die();
}

/**
 * @param array $form
 * @param array $form_state
 */
function send_statistics($form, &$form_state)
{
    global $base_url;
    $path        = drupal_get_path('module', 'yamoney_api').'/'.'yamoney_api.info';
    $info        = drupal_parse_info_file($path);
    $headers     = array();
    $headers[]   = 'Content-Type: application/x-www-form-urlencoded';
    $form_data   = $form_state['input'];
    $array       = array(
        'url'      => $base_url,
        'cms'      => 'drupal',
        'version'  => VERSION,
        'ver_mod'  => $info['version'],
        'yacms'    => false,
        'email'    => variable_get('site_mail', ''),
        'shopid'   => (isset($form_data['yamoney_api_shop_id'])) ? $form_data['yamoney_api_shop_id'] : 0,
        'settings' => array(
            'kassa'     => (isset($form_data['yamoney_api_shop'])) ? true : false,
            'kassa_epl' => (isset($form_data['yamoney_api_paymode']) && $form_data['yamoney_api_paymode'] == 'kassa') ? true : false,
            'p2p'       => (isset($form_data['yamoney_api_shop'])) ? false : true,
        ),
    );
    $array_crypt = base64_encode(serialize($array));

    $url     = 'https://statcms.yamoney.ru/v2/';
    $curlOpt = array(
        CURLOPT_HEADER         => false,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false,
        CURLINFO_HEADER_OUT    => true,
        CURLOPT_POST           => true,
    );

    $curlOpt[CURLOPT_HTTPHEADER] = $headers;
    $curlOpt[CURLOPT_POSTFIELDS] = http_build_query(array('data' => $array_crypt, 'lbl' => 1));

    $curl = curl_init($url);
    curl_setopt_array($curl, $curlOpt);
    $rbody = curl_exec($curl);
    $errno = curl_errno($curl);
    $error = curl_error($curl);
    $rcode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    curl_close($curl);
}


function getUcOrderStatuses()
{
    $query = db_query("SELECT * FROM {uc_order_statuses}");

    return $query->fetchAllKeyed();
}

function remove_duplicate_messages($type)
{
    $stack = array();
    if (isset($_SESSION['messages'][$type])) {
        foreach ($_SESSION['messages'][$type] as $k => $msg) {
            if (in_array($msg, $stack)) {
                // Message is already set - remove it
                unset($_SESSION['messages'][$type][$k]);
            } else {
                // Add message to stack
                $stack[] = $msg;
            }
        }
    }
}