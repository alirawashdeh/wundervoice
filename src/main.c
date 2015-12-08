#include <pebble.h>

// Configuration
#define KEY_CONFIGURED 0
#define KEY_SUCCESS 1
#define KEY_MESSAGE 2
#define KEY_LIST 3
static bool configured;

// UI
static Window *s_main_window;
static TextLayer *s_text_layer;
static TextLayer *title_banner;
static TextLayer *s_text_sublayer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

// Timer
static AppTimer *s_timer;

// Dictation
static DictationSession *s_dictation_session;
static char s_last_text[512];
static char s_added_to[128];
static char s_task_in_quotes[128];

/***** Click Handlers *****/
static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  // Start voice dictation UI
  dictation_session_start(s_dictation_session);
}

static void click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/***** Callbacks *****/
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
  char *transcription, void *context)
  {
    if(status == DictationSessionStatusSuccess)
    {
      // Success - Send transcription to PebbleKit Javascript to add to Wunderlist
      strcpy(s_last_text,transcription);
      DictionaryIterator *iterout;
      app_message_outbox_begin(&iterout);
      dict_write_cstring(iterout, KEY_MESSAGE, transcription);
      app_message_outbox_send();
    }
    else
    {
      if(status == DictationSessionStatusFailureTranscriptionRejected)
      {
        // Cancelled - Show message and set up click handler for retry
        text_layer_set_text(s_text_layer, "Cancelled");
        text_layer_set_text(s_text_sublayer, "Press select to try again.");
        window_set_click_config_provider(s_main_window, click_config_provider);
      }
      else
      {
        // Error - Log and show error. Set up click handler for retry.
        static char s_failed_buff[128];
        snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\nError:%d", (int)status);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_failed_buff);
        text_layer_set_text(s_text_layer, s_failed_buff);
        text_layer_set_text(s_text_sublayer, "Press select to try again.");
        window_set_click_config_provider(s_main_window, click_config_provider);
      }
    }
  }

  static void timer_callback(void *data)
  {
    // Completion message shown for a few seconds, so close app.
    window_stack_pop_all(true);
  }

  /***** Messaging Event Handlers *****/
  static void inbox_received_handler(DictionaryIterator *iter, void *context)
  {
    Tuple *configured = dict_find(iter, KEY_CONFIGURED);
    if(configured != NULL)
    {
      // Application configured event, so show message and set click handler
      persist_write_bool(KEY_CONFIGURED, 1);
      text_layer_set_text(s_text_layer, "Configuration Complete");
      text_layer_set_text(s_text_sublayer, "Press select to start dictating");
      window_set_click_config_provider(s_main_window, click_config_provider);
    }
    else
    {
      // Check for response from Wunderlist
      Tuple *response = dict_find(iter, KEY_SUCCESS);
      if(response != NULL)
      {
        if(response->value->int32 == 1)
        {
          // Success - Show message and set a timer.
          snprintf(s_task_in_quotes, sizeof(s_task_in_quotes), "\"%s\"", s_last_text);
          text_layer_set_text(s_text_layer, s_task_in_quotes);
          s_timer = app_timer_register(4000, timer_callback, NULL);
          vibes_short_pulse();
          Tuple *listname = dict_find(iter, KEY_LIST);
          if(listname != NULL)
          {
            // If we have the list name, tell the user what list it was added to.
            snprintf(s_added_to, sizeof(s_added_to), "Added to %s", listname->value->cstring);
            text_layer_set_text(s_text_sublayer, s_added_to);
          }
        }
        else
        {
          // Failure response from Wunderlist - show message and set click handler.
          text_layer_set_text(s_text_layer, "Something went wrong.");
          text_layer_set_text(s_text_sublayer, "Press select to try again.");
          window_set_click_config_provider(s_main_window, click_config_provider);
        }
      }
    }
  }

  static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  }

  static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success - task add");
    text_layer_set_text(s_text_layer, "Adding task");
    text_layer_set_text(s_text_sublayer, "Please be patient.");
  }

  /***** Application methods *****/
  static void window_load(Window *window)
  {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Draw banner
    title_banner = text_layer_create(GRect(0, 0, bounds.size.w, 36));
    text_layer_set_background_color(title_banner, GColorRed);
    layer_add_child(window_layer, text_layer_get_layer(title_banner));

    // Draw text
    s_text_layer = text_layer_create(GRect(5, 46, bounds.size.w - 10, 40));
    text_layer_set_text(s_text_layer, "Configuration Required");
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
    text_layer_set_text_color(s_text_layer,GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

    // Draw subtext
    s_text_sublayer = text_layer_create(GRect(5, 86, bounds.size.w -10, bounds.size.h - 86));
    text_layer_set_text(s_text_sublayer, "Open app settings on your phone to configure.");
    text_layer_set_font(s_text_sublayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_text_sublayer, GTextAlignmentCenter);
    text_layer_set_text_color(s_text_sublayer,GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_text_sublayer));

    // Draw Icon
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STAR);
    s_bitmap_layer = bitmap_layer_create(GRect((bounds.size.w/2) -16, 3, 32, 32));
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  }

  static void window_unload(Window *window)
  {
    text_layer_destroy(title_banner);
    text_layer_destroy(s_text_layer);
    text_layer_destroy(s_text_sublayer);
    gbitmap_destroy(s_bitmap);
    bitmap_layer_destroy(s_bitmap_layer);
  }

  static void init()
  {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers)
    {
      .load = window_load,
      .unload = window_unload,
    });
    window_stack_push(s_main_window, true);
    s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);

    configured = persist_read_bool(KEY_CONFIGURED);
    if(configured)
    {
      dictation_session_start(s_dictation_session);
    }
    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  }

  static void deinit()
  {
    window_destroy(s_main_window);
    dictation_session_destroy(s_dictation_session);
  }

  int main() {
    init();
    app_event_loop();
    deinit();
  }
