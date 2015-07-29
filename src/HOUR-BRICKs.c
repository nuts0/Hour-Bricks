#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

static Window *s_main_window;
static Layer *display_layer;
TextLayer *text_layer;

char buffer[] = "00";

static Layer *s_date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_day_label, *s_num_label, *s_month_label;
static char s_day_buffer[6],s_num_buffer[4],s_month_buffer[4];

#define Hbrick_WIDTH 15
#define Hbrick_HEIGHT 6
#define Vbrick_WIDTH 6
#define Vbrick_HEIGHT 19
#define brick_PADDING 6

//------------------------------------------------------------------------------------
static void handle_battery(BatteryChargeState charge_state) {
	static char battery_text[] = "100%";

	if (charge_state.is_charging) {
		snprintf(battery_text, sizeof(battery_text), "chg");
	} else {
		snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
	}
	text_layer_set_text(s_battery_layer, battery_text);
}
//------------------------------------------------------------------------------------
GRect brick_location(int H) {
	int cx,cy,brick_H,brick_W;
	cx=0;cy=0;brick_H=0;brick_W=0;
	
	if(H==0){
		cx=brick_PADDING*3;
		cy=164-Hbrick_HEIGHT;
		brick_H=Hbrick_HEIGHT;
		brick_W=Hbrick_WIDTH;
	}else if(H>0 && H<=6){
		cx=brick_PADDING;
		cy=164-H*(Vbrick_HEIGHT+brick_PADDING)+brick_PADDING;
		brick_H=Vbrick_HEIGHT;
		brick_W=Vbrick_WIDTH;
	}else if(H>6 && H<=12){
		cx=(H-7)*(Hbrick_WIDTH)+(H-6)*brick_PADDING;
		cy=brick_PADDING;
		brick_H=Hbrick_HEIGHT;
		brick_W=Hbrick_WIDTH;
	}else if(H>12 && H<=18){
		cx=144-brick_PADDING-Vbrick_WIDTH;
		cy=(H-13)*Vbrick_HEIGHT+(H-12)*brick_PADDING;
		brick_H=Vbrick_HEIGHT;
		brick_W=Vbrick_WIDTH;
	}else if(H>18 && H<=23 ){
		cx=144-((H-18)*Hbrick_WIDTH+(H-18)*brick_PADDING);
		cy=164-brick_PADDING;
		brick_H=Hbrick_HEIGHT;
		brick_W=Hbrick_WIDTH;
	}
  return GRect(cx,cy, brick_W, brick_H);
}
//------------------------------------------------------------------------------------
void display_layer_update_callback(Layer *layer, GContext* ctx) {
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);

	int hour = tick_time->tm_hour;

	graphics_context_set_fill_color(ctx, GColorLightGray);
	for (int i = 1; i < hour; ++i) {
		graphics_fill_rect(ctx, brick_location(i), 1, GCornersAll);
	}
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, brick_location(hour), 0, GCornersAll);

	//BATT update
	handle_battery(battery_state_service_peek());
}
//------------------------------------------------------------------------------------
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(display_layer);

	//Format the buffer string using tick_time as the time source
	strftime(buffer, sizeof("00"), "%M", tick_time);
	//Change the TextLayer text to show the new time!
	text_layer_set_text(text_layer, buffer);
}
//------------------------------------------------------------------------------------
static void date_update_proc(Layer *layer, GContext *ctx) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	//month
	strftime(s_month_buffer, sizeof(s_month_buffer), "%b", t);
	text_layer_set_text(s_month_label, s_month_buffer);
	//曜日
	strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
	text_layer_set_text(s_day_label, s_day_buffer);
	//日付
	strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
	text_layer_set_text(s_num_label, s_num_buffer);

}
//------------------------------------------------------------------------------------
static void window_load(Window *window) {
	//Time layer
	text_layer = text_layer_create(GRect(30, 22, 90, 80));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, custom_font);
//	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));

	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);	

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	display_layer = layer_create(bounds);
	layer_set_update_proc(display_layer, display_layer_update_callback);
	layer_add_child(window_layer, display_layer);
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	layer_mark_dirty(display_layer);

	GRect window_bounds = layer_get_bounds(window_layer);
	// youbi 	
	s_date_layer = layer_create(window_bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_day_label = text_layer_create(GRect(30, 75, 90,105));
	text_layer_set_text(s_day_label, s_day_buffer);
	text_layer_set_background_color(s_day_label, GColorClear);
	text_layer_set_text_color(s_day_label, GColorWhite);
	text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);
	text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

	// date 
	s_num_label = text_layer_create(GRect(30, 95, 90, 129));
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_background_color(s_num_label, GColorClear);
	text_layer_set_text_color(s_num_label, GColorWhite);
	text_layer_set_text_alignment(s_num_label, GTextAlignmentCenter);
	text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

	//month
	s_month_label = text_layer_create(GRect(30, 115, 90, 153));
	text_layer_set_text(s_month_label, s_month_buffer);
	text_layer_set_background_color(s_month_label, GColorClear);
	text_layer_set_text_color(s_month_label, GColorWhite);
	text_layer_set_text_alignment(s_month_label, GTextAlignmentCenter);
	text_layer_set_font(s_month_label, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(s_date_layer, text_layer_get_layer(s_month_label));

	/* Battery */	
	s_battery_layer = text_layer_create(GRect(30, 135, 90, 163));
	text_layer_set_text_color(s_battery_layer, GColorWhite);
	text_layer_set_background_color(s_battery_layer, GColorClear);
	text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
	text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text(s_battery_layer, "100%");

	battery_state_service_subscribe(handle_battery);
//	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

}
//------------------------------------------------------------------------------------
static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	layer_destroy(display_layer);
	battery_state_service_unsubscribe();
	text_layer_destroy(s_battery_layer);
	text_layer_destroy(s_month_label);
	text_layer_destroy(s_day_label);
	text_layer_destroy(s_num_label);
}
//------------------------------------------------------------------------------------
static void init() {
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
    	.load = window_load,
    	.unload = window_unload,
	});

	/*BLACK back*/
	window_set_background_color(s_main_window, COLOR_FALLBACK(GColorBlack, GColorBlack));

	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_Titillium_BOLD_60));
//	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_Titillium_BoldUpright_60));
	
	window_stack_push(s_main_window, true);
}
//------------------------------------------------------------------------------------
static void deinit() {
	tick_timer_service_unsubscribe();
	window_destroy(s_main_window);
}
//------------------------------------------------------------------------------------
int main(void) {
	init();
	app_event_loop();
	deinit();
}
