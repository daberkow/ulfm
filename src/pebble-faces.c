#include <pebble.h>
#include "netdownload.h"
#ifdef PBL_PLATFORM_APLITE
  #include "png.h"
#endif
#include "pebble-faces.h"

#define STORED_USER_TOKEN_LOC 10

static Window *window;
static TextLayer *text_layer;
static Layer *global_layer;
static GBitmap *current_bmp;
static GPath *s_minute_arrow;
static BitmapLayer *lastLogo;

void show_logo_image(ClickRecognizerRef recognizer, void *context) {
  // show that we are loading by showing no image
  bitmap_layer_set_bitmap(lastLogo, NULL);
  // Unload the current image if we had one and save a pointer to this one
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
    current_bmp = NULL;
  }

  netdownload_request("http://192.168.1.110/last.pbl.png");
}

//Draws the cornered rectangles
static void drawRounded(Layer *layer, GContext *ctx)
{
  GRect dan = layer_get_bounds(layer);
  //The if is here incase I wantd different colors
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, dan, 8, GCornersAll);
  #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, dan, 8, GCornersAll);
  #endif
}

//144, 168
static void win_load_need_setup(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_PLATFORM_BASALT
    window_set_background_color(window, GColorDarkCandyAppleRed);
    window_stack_push(window, true);
  #else
    window_set_background_color(window, GColorBlack);
    window_stack_push(window, true);
  #endif

  global_layer = layer_create(GRect(22, 20, 100, 60));
  layer_set_update_proc(global_layer, drawRounded);
  layer_add_child(window_layer, global_layer);

  text_layer = text_layer_create((GRect) { .origin = { 8, 4 }, .size = { 84, 40 } });
  text_layer_set_text(text_layer, "Unoffical");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(global_layer, text_layer_get_layer(text_layer));

  text_layer = text_layer_create((GRect) { .origin = { 8, 19 }, .size = { 84, 40 } });
  text_layer_set_text(text_layer, "Last.fm");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(global_layer, text_layer_get_layer(text_layer));

  global_layer = layer_create(GRect(22, 100, 100, 60));
  layer_set_update_proc(global_layer, drawRounded);
  layer_add_child(window_layer, global_layer);

  text_layer = text_layer_create((GRect) { .origin = { 8, 0 }, .size = { 84, 60 } });
  text_layer_set_text(text_layer, "Go To Settings In The Pebble App To Login!");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(global_layer, text_layer_get_layer(text_layer));
}


static void win_unload_need_setup(Window *window) {
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(lastLogo);
  gbitmap_destroy(current_bmp);
}

void download_complete_handler(NetDownload *download) {
  printf("Loaded image with %lu bytes", download->length);
  printf("Heap free is %u bytes", heap_bytes_free());

  #ifdef PBL_PLATFORM_APLITE
  GBitmap *bmp = gbitmap_create_with_png_data(download->data, download->length);
  #else
    GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
  #endif
  bitmap_layer_set_bitmap(lastLogo, bmp);

  // Save pointer to currently shown bitmap (to free it)
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
  }
  current_bmp = bmp;

  // Free the memory now
  #ifdef PBL_PLATFORM_APLITE
  // gbitmap_create_with_png_data will free download->data
  #else
    free(download->data);
  #endif
  // We null it out now to avoid a double free
  download->data = NULL;
  netdownload_destroy(download);
}

void config_provider(Window *window) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_DOWN, show_logo_image);
}

static void init(void) {
  // Need to initialize this first to make sure it is there when
  // the window_load function is called by window_stack_push.
  netdownload_initialize(download_complete_handler);

  window = window_create();
  #ifdef PBL_SDK_2
    window_set_fullscreen(window, true);
  #endif

  if (persist_exists(STORED_USER_TOKEN_LOC))
  {

  }else{
    window_set_window_handlers(window, (WindowHandlers) {
      .load = win_load_need_setup,
      .unload = win_unload_need_setup,
    });
  }

  const bool animated = true;
  window_stack_push(window, animated);
  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
}

static void deinit(void) {
  netdownload_deinitialize(); // call this to avoid 20B memory leak
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
