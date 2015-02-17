#include <pebble.h>
#include <stdlib.h>

static Window *window;
static TextLayer *text_layer;
static Layer *s_canvas_layer;
static int play_speed = 0;
// 0 = paused
// 1 = 2000 ms
// 2 = 1000 ms
// 3 = 500 ms

static int cell_size = 2;
// 144*152 is the hardcoded size of the pebble screen.
// 8 bit buffer added to make sure we get to the next byte.
// Divided by 8 because each char can store 8 bits.
static char board[8+(144*152)/8];
static char new_board[8+(144*152)/8];
static int board_width = 24;
static int board_height = 25;
static AppTimer *app_timer;
static AppTimer *text_timer;
// cell sizes affect grid size
// 0 = 72x76
// 1 = 48x50
// 2 = 24x25
// 3 = 16x16
// 4 = 144X152

// Function signatures.
static void set_playback_timer();


// Accessors for the boolean values in the char array.
static bool get_board_value(int index)
{
  int packed_index = index/8;
  int offset_value = index%8;
  char mask_value = 1 << offset_value;
  char cur_char = board[packed_index];
  return ((cur_char & mask_value) != 0);
}
static void set_board_value(int index, bool reg_board, bool value)
{
  int packed_index = index/8;
  int offset_value = index%8;
  char mask_value = 1 << offset_value;
  if(value == false)
  {
    mask_value = mask_value ^ 255;
  }
  if(reg_board)
  {
    char cur_char = board[packed_index];
    if(value == true)
    {
      board[packed_index] = mask_value | cur_char;
    }
    else
    {
      board[packed_index] = mask_value & cur_char;
    }
  }
  else
  {
    char cur_char = new_board[packed_index];
    if(value == true)
    {
      new_board[packed_index] = mask_value | cur_char;
    }
    else
    {
      new_board[packed_index] = mask_value & cur_char;
    }

  }
}

static int get_neighbor_count(int i, int j)
{
  int neighbor_count = 0;
  for(int x = i-1; x <= i+1; x++)
  {
    for(int y = j-1; y <= j+1; y++)
    {
      if(y == j && x == i)//skip the cell itself.
      {
        continue;
      }
      if (x < 0 || x >= board_width || y < 0 || y >= board_height)
      {
        continue;
      }
      int index = x + (y*board_width);
      if(get_board_value(index) == true)
      {
        neighbor_count++;
        if(neighbor_count > 3)
        {
          return neighbor_count;
        }
      }
    }
  }
  return neighbor_count;
}

static void step_board()
{
  for(int i = 0; i < board_width; i++)
  {
    for(int j = 0; j < board_height; j++)
    {
      int index = i + (j*board_width);
      int neighbors = get_neighbor_count(i,j);
      bool new_value = ( (neighbors == 2 && get_board_value(index) == true) || (neighbors == 3));
      set_board_value(index,false,new_value);
    }
  }
  memcpy(board, new_board, sizeof(char[8+(board_height*board_width)/8]));
}

static void trigger_automatic_playback(void *data){
  step_board();
  set_playback_timer();
  layer_mark_dirty(s_canvas_layer);
}

static void set_playback_timer()
{
  if(play_speed == 0)
  {
    //paused
  }
  if(play_speed == 1)
  {
    app_timer = app_timer_register(1000, trigger_automatic_playback, NULL);
  }
  if(play_speed == 2)
  {
    app_timer = app_timer_register(500, trigger_automatic_playback, NULL);
  }
  if(play_speed == 3)
  {
    app_timer = app_timer_register(250, trigger_automatic_playback, NULL);
  }
}

static void toggle_play_speed(){
    if(app_timer != NULL)
    {
      app_timer_cancel(app_timer);
    }
    play_speed++;
    if(play_speed > 3)
    {
      play_speed = 0;
    }
    set_playback_timer();
}

static void toggle_cell_size(){
  cell_size++;
  if(cell_size > 4)
  {
    cell_size = 0;
    board_width = 72;
    board_height = 76;
  }
  if(cell_size == 1)
  {
    board_width = 48;
    board_height = 50;
  }
  if(cell_size == 2)
  {
    board_width = 24;
    board_height = 25;
  }
  if(cell_size == 3)
  {
    board_width = 16;
    board_height = 16;
  }
  if(cell_size == 4)
  {
    board_width = 144;
    board_height = 152;
  }
}

static void generate_random_board(){
  srand(time(NULL));
  // Create a board of the appropriate size for the cell_size
  int array_size = board_width * board_height;

  for(int i = 0; i < array_size; i++)
  {
    bool new_val = ( (rand() % 2) == 1);
    set_board_value(i,true,new_val);
  }
}


static void write_board_to_layer(Layer *this_layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorBlack);

  for(int i = 0; i < board_width; i++)
  {
    for(int j = 0; j < board_height; j++)
    {
      int index = i + (j*board_width);
      if(get_board_value(index) == true)
      {
        if(cell_size == 0)
        {
          graphics_fill_rect(ctx, GRect(i*2, j*2, 2, 2), 0, GCornerNone);
        }
        else if(cell_size == 1)
        {
          graphics_fill_rect(ctx, GRect(i*3, j*3, 3, 3), 0, GCornerNone);
        }
        if(cell_size == 2)
        {
          graphics_fill_rect(ctx, GRect(i*6, j*6, 6, 6), 1, GCornersAll);
        }
        if(cell_size == 3)
        {
          graphics_fill_rect(ctx, GRect(i*9, j*9, 9, 9), 4, GCornersAll);
        }
        if(cell_size == 4)
        {
          graphics_fill_rect(ctx, GRect(i, j, 1, 1), 0, GCornerNone);
        }
      }
    }
  }
}


static void clear_text_color() {
  text_layer_set_text(text_layer, "");
  text_layer_set_text_color(text_layer, GColorClear);
  text_layer_set_background_color(text_layer, GColorClear);
  layer_mark_dirty(text_layer_get_layer(text_layer));
}
static void reset_text_color() {
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorWhite);
}

static void set_text_clear_timer() {
    reset_text_color();
    if(text_timer != NULL)
    {
      app_timer_cancel(text_timer);
    }
    text_timer = app_timer_register(700, clear_text_color, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Random!");
  set_text_clear_timer();
  generate_random_board();
  layer_mark_dirty(s_canvas_layer);
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  toggle_play_speed();
  step_board();
  char *speed; 
  if(play_speed == 0)
  {
    //paused
    speed = "Paused";
  }
  if(play_speed == 1)
  {
    speed = "Slow...";
  }
  if(play_speed == 2)
  {
    speed = "Fast.";
  }
  if(play_speed == 3)
  {
    speed = "Fastest!";
  }
  text_layer_set_text(text_layer, speed);
  set_text_clear_timer();
  layer_mark_dirty(s_canvas_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "");
  toggle_cell_size();
  generate_random_board();
  layer_mark_dirty(s_canvas_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Create Life Layer
  generate_random_board();
  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);

  // Create Text Layer
  text_layer = text_layer_create((GRect) { .origin = { bounds.size.w/4, 0 }, .size = { bounds.size.w/2, 20 } });
  clear_text_color();
  text_layer_set_text(text_layer, "");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  // Set the update_proc
  layer_set_update_proc(s_canvas_layer, write_board_to_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  layer_destroy(s_canvas_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
