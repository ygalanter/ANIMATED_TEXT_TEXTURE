/*
 * main.c
 * Sets up a Window, BitmapLayer and blank GBitmap to be used as the display
 * container for the GBitmapSequence. It also counts the number of frames.
 *
 * Animation source:
 * http://bestanimations.com/Science/Physics/Physics2.html
 */

#include <pebble.h>

static Window *s_main_window;

static Layer *n_layer;

static GBitmap *s_bitmap = NULL;
static GBitmapSequence *s_sequence = NULL;

static int frame_counter = 0;

static void load_sequence();

static void update_n_layer(Layer *layer, GContext *ctx) {
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, "08:39", fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(0,50,144,118), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  GBitmap *fb = graphics_capture_frame_buffer_format(ctx, GBitmapFormat8Bit);
  
  uint64_t *fb_data = (uint64_t *)gbitmap_get_data(fb);
  uint64_t *anim_data =  (uint64_t *)gbitmap_get_data(s_bitmap);
  
  for (int i=50*144/8; i < 100*144/8; i++) {
     fb_data[i] &= anim_data[i];
  }  
  
  graphics_release_frame_buffer(ctx, fb);
   
}
  

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    
    layer_mark_dirty(n_layer);
    
    // Timer for that delay
    app_timer_register(next_delay, timer_handler, NULL);

    frame_counter++;
  } else {
    // Start again
    load_sequence();

    APP_LOG(APP_LOG_LEVEL_INFO, "Frames: %d", frame_counter);
    frame_counter = 0;
  }
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
    
  n_layer = layer_create(bounds);
  layer_set_update_proc(n_layer, update_n_layer);
  layer_add_child(window_layer, n_layer);

  load_sequence();
}

static void main_window_unload(Window *window) {
  gbitmap_sequence_destroy(s_sequence);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_fullscreen(s_main_window, true);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}