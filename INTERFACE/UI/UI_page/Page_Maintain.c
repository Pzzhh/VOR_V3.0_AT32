#include "Page_Maintain.h"
#include "../UI_Include.h"
#include "../other/meassage_center.h"

enum
{
	LV_MENU_ITEM_BUILDER_VARIANT_1,
	LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;

static void back_event_handler(lv_event_t* e);
// static void switch_handler(lv_event_t* e);
lv_obj_t* root_page;
static lv_obj_t* create_text(lv_obj_t* parent, const char* icon, const char* txt,
	lv_menu_builder_variant_t builder_variant);
static lv_obj_t* create_slider(lv_obj_t* parent,
	const char* icon, const char* txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t* create_switch(lv_obj_t* parent,
	const char* icon, const char* txt, bool chk);

/*HANDLE CAM*/ static void Camera_Rec_sw_handle(lv_event_t* e)
{
	lv_obj_t* sw = e->current_target;
	uint8_t isclick = lv_obj_has_state(sw, LV_STATE_CHECKED);
	if (Message_Center_Read_prinft("Ctrl", 0, 0,
		"Camere Rec", !isclick) == 0)
	{
		if (isclick)
			lv_obj_clear_state(sw, LV_STATE_CHECKED);
		else
			lv_obj_add_state(sw, LV_STATE_CHECKED);
	}
}

/*HANDLE CAM*/ static void Camera_WIFI_btn_handle(lv_event_t* e)
{
	lv_obj_t* btn = e->current_target;
	char msg[10] = { 0 };
	uint8_t state = 1;
	if (Message_Center_Read_prinft("Ctrl", msg, sizeof(msg),
		"Camere wifi") == 0)
	{
		state = 0;
		if (strncmp(msg, "Fail", 4) == 0)
		{
			state = 2;
		}
	}
	if (state != 0)
	{
		lv_obj_t* mbox1 = lv_msgbox_create(NULL, "error message",
			(state == 1) ? "CMD FAIL" : (state == 2) ? "CAN connect FAIL"
			: "Reserved",
			0, true);
		lv_obj_center(mbox1);
	}
}

/*HANDLE CAM*/ static void Camera_vol_slider_handle(lv_event_t* e)
{
	const uint32_t offsetVal = 1000;
	lv_obj_t* slider = e->current_target;
	char msg[20] = { 0 };
	uint8_t state = 1;
	uint32_t sliderVal = lv_slider_get_value(slider);
	uint32_t VolSet = sliderVal * 5 + 1000;
	lv_label_set_text_fmt(slider->user_data, "%01d.%03d", VolSet / 1000, VolSet % 1000);
	if (Message_Center_Read_prinft("Ctrl", msg, sizeof(msg),
		"Camere Vol %d", VolSet) == 0)
	{
		state = 0;
		if (strncmp(msg, "Fail", 4) == 0)
		{
			state = 2;
		}
	}
	if (state != 0)
	{
		/*lv_obj_t* mbox1 = lv_msgbox_create(NULL, "error message",
			(state == 1) ? "CMD FAIL" : (state == 2) ? "CAN connect FAIL"
			: "Reserved",
			0, true);
		lv_obj_center(mbox1);*/
	}
}

/*HANDLE FAN*/ static void FAN_vol_slider_handle(lv_event_t* e)
{
	lv_obj_t* slider = e->current_target;
	uint32_t sliderVal = lv_slider_get_value(slider);
	uint32_t VolSet = sliderVal;
	lv_label_set_text_fmt(slider->user_data, "%d", VolSet);
	if (Message_Center_Read_prinft("Ctrl", 0, 0,
		"FAN_pwm %d", VolSet) == 0)
	{

	}
}

void UI_page_maintain_incline_Up_handle(lv_event_t* e)
{
	if (e->code == LV_EVENT_PRESSING)
	{
		Message_Center_Read_prinft("Ctrl", 0, 0,
			"Inc Up");
	}
	if (e->code == LV_EVENT_RELEASED)
	{
		Message_Center_Read_prinft("Ctrl", 0, 0,
			"Inc stop");
	}
}

void UI_page_maintain_incline_down_handle(lv_event_t* e)
{
	if (e->code == LV_EVENT_PRESSING)
	{
		Message_Center_Read_prinft("Ctrl", 0, 0,
			"Inc down");
		// printf("...");
	}
	if (e->code == LV_EVENT_RELEASED)
	{
		Message_Center_Read_prinft("Ctrl", 0, 0,
			"Inc stop");
		// printf("|");
	}
}

void UI_page_maintain_incline_init(lv_obj_t* parent)
{
	lv_obj_t* btn = lv_btn_create(parent);
	lv_obj_t* label = lv_label_create(btn);
	lv_label_set_text(label, LV_SYMBOL_UP);

	lv_obj_add_event_cb(btn, UI_page_maintain_incline_Up_handle, LV_EVENT_ALL, 0);

	btn = lv_btn_create(parent);
	label = lv_label_create(btn);
	lv_label_set_text(label, LV_SYMBOL_DOWN);

	lv_obj_add_event_cb(btn, UI_page_maintain_incline_down_handle, LV_EVENT_ALL, 0);
}

static void anim_y_cb(void* var, int32_t v)
{

	lv_obj_t* label = var;
	if (lv_obj_is_visible(var))
	{
		float angle = 0;
		HAL_IMU_GET_Angle(&angle);
		lv_label_set_text_fmt(label, "%d.%02d", (int)angle, (int)(angle * 100.0f) % 100);
	}
	// lv_obj_set_y((lv_obj_t*)var, v);
}

void lv_example_menu_55(lv_obj_t* parent, lv_obj_t* obj)
{
	/*Create a list*/
	lv_obj_t* list1 = lv_list_create(parent);
	lv_obj_set_size(list1, 400, 400);
	lv_obj_center(list1);

	/*Add buttons to the list*/
	lv_obj_t* btn;

	lv_list_add_text(list1, "Camera");
	// lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "Rec");
	lv_obj_add_event_cb(btn,
		Camera_Rec_sw_handle, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_btn(list1, LV_SYMBOL_WIFI, "wifi");
	lv_obj_add_event_cb(btn,
		Camera_WIFI_btn_handle, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_btn(list1, LV_SYMBOL_CHARGE, "IR led voltage\n");
	lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(create_slider(btn, 0, 0, 0, 80, 50),
		Camera_vol_slider_handle, LV_EVENT_VALUE_CHANGED, NULL);

	lv_list_add_text(list1, "incline");
	btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "manual control");
	lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
	UI_page_maintain_incline_init(btn);
	// lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "imu data");
	lv_obj_t* label = lv_label_create(btn);
	lv_anim_t a, * anim;
	lv_anim_init(&a);
	// a.user_data = table_proprity;
	lv_anim_set_var(&a, label);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_exec_cb(&a, anim_y_cb); // 设置执行回调函数
	// lv_anim_set_path_cb(&a, custom_anim_path); // 设置自定义路径回调函数
	anim = lv_anim_start(&a); // 开始动画
	// lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

	lv_list_add_text(list1, "motion");
	btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "IR ");
	// lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "servo ctrl");

	lv_list_add_text(list1, "other");
	btn = lv_list_add_btn(list1, 0, "FAN ");
	lv_obj_add_event_cb(create_slider(btn, 0, 0, 0, 100, 50),
		FAN_vol_slider_handle, LV_EVENT_VALUE_CHANGED, NULL);
	// lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
	// lv_list_add_text(list1, "date");
	// btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "time");
}

static void back_event_handler(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	lv_obj_t* menu = lv_event_get_user_data(e);

	if (lv_menu_back_btn_is_root(menu, obj))
	{
		lv_obj_t* mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
		lv_obj_center(mbox1);
	}
}

static lv_obj_t* create_text(lv_obj_t* parent, const char* icon, const char* txt,
	lv_menu_builder_variant_t builder_variant)
{
	lv_obj_t* obj = (lv_obj_t*)lv_menu_cont_create(parent);

	lv_obj_t* img = NULL;
	lv_obj_t* label = NULL;

	if (icon)
	{
		img = lv_img_create(obj);
		lv_img_set_src(img, icon);
	}

	if (txt)
	{
		label = lv_label_create(obj);
		lv_label_set_text(label, txt);
		lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
		lv_obj_set_flex_grow(label, 1);
	}

	if (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt)
	{
		lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
		lv_obj_swap(img, label);
	}

	return obj;
}

static lv_obj_t* create_slider(lv_obj_t* parent, const char* icon, const char* txt, int32_t min, int32_t max,
	int32_t val)
{
	// lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);
	lv_obj_t* slider = lv_slider_create(parent);
	slider->user_data = lv_label_create(parent);
	lv_label_set_text(slider->user_data, "voltage");
	// lv_obj_move_to(slider, 0, 50);
	// lv_obj_set_flex_grow(slider, 1);
	lv_slider_set_range(slider, min, max);
	lv_slider_set_value(slider, val, LV_ANIM_OFF);

	if (icon == NULL)
	{
		lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
	}

	return slider;
}

static lv_obj_t* create_switch(lv_obj_t* parent, const char* icon, const char* txt, bool chk)
{
	lv_obj_t* obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

	lv_obj_t* sw = lv_switch_create(obj);
	lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

	return obj;
}
