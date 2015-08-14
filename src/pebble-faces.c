#include <pebble.h>
#include "netdownload.h"
#ifdef PBL_PLATFORM_APLITE
  #include "png.h"
#endif
#include "pebble-faces.h"

#define STORED_USER_TOKEN_LOC 10


static Window *window;
static TextLayer *text_layer;
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

// .update_proc of my_layer:
static void my_layer_update_proc(Layer *my_layer, GContext* ctx) {
  // Fill the path:
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_filled(ctx, s_minute_arrow);
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, GColorBlack);
  //gpath_draw_outline(ctx, s_minute_arrow);
}

static void my_layer_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw a black filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw a white filled circle a radius of half the layer height
  //graphics_context_set_fill_color(ctx, GColorWhite);
  //const int16_t half_h = bounds.size.h / 2;
  //graphics_fill_circle(ctx, GPoint(half_h, half_h), half_h);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_PLATFORM_BASALT
    window_set_background_color(window, GColorDarkCandyAppleRed);
    window_stack_push(window, true);
  #endif



  //lastLogo = bitmap_layer_create(GRect(48, 20, 48, 48));
  //layer_add_child(window_layer, bitmap_layer_get_layer(lastLogo));

  text_layer = text_layer_create((GRect) { .origin = { 20, 30 }, .size = { 100, 40 } });
  text_layer_set_text(text_layer, "Unoffical");
  //text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer = text_layer_create((GRect) { .origin = { 20, 45 }, .size = { 100, 40 } });
  text_layer_set_text(text_layer, "Last.fm");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));


  text_layer = text_layer_create((GRect) { .origin = { 20, 110 }, .size = { 100, 40 } });
  text_layer_set_text(text_layer, "Go To Settings To Login!");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  current_bmp = NULL;
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
      .load = window_load,
      .unload = window_unload,
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
