#include "PageManagment.h"

#include "../UI_Include.h"

#include "page1.h"
#include "Page_Maintain.h"

//烧录
//硬件配置（灯光 相机wifi 倾斜 陀螺仪 光电 错误历史 内存占用 ）
//测试模组
void UI_Page_Management_Init()
{


	//lv_obj_t* parent = lv_scr_act();
	////Page1_init(parent);
	//lv_example_menu_32();

	/*Create a menu object*/
	lv_obj_t* menu = lv_menu_create(lv_scr_act());
	lv_obj_set_size(menu, 800, 480);
	lv_obj_center(menu);
	lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
	lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);


	/*Modify the header*/
	lv_obj_t* back_btn = lv_menu_get_main_header_back_btn(menu);
	lv_obj_t* back_btn_label = lv_label_create(back_btn);
	lv_label_set_text(back_btn_label, "Back");

	lv_obj_t* cont;
	lv_obj_t* label;

	/*Create sub pages*/
	lv_obj_t* sub_1_page = lv_menu_page_create(menu, "");
	//lv_obj_set_size(sub_1_page, 800, 430);
	lv_obj_t* obj = lv_obj_create(sub_1_page);
	lv_obj_set_size(obj, 795, 430);
	Page1_init(obj);




	/*cont = lv_menu_cont_create(sub_1_page);
	label = lv_label_create(cont);
	lv_label_set_text(label, "Hello, I am hiding here");*/

	lv_obj_t* sub_2_page = lv_menu_page_create(menu, "Settings");

	extern void lv_example_menu_55(lv_obj_t * parent, lv_obj_t * obj);
	lv_example_menu_55(sub_2_page, menu);

	/*lv_obj_t* sub_3_page = lv_menu_page_create(menu, "Page 3");

	cont = lv_menu_cont_create(sub_3_page);
	label = lv_label_create(cont);
	lv_label_set_text(label, "Hello, I am hiding here");*/

	lv_obj_t* setpageCont;
	/*Create a main page*/
	lv_obj_t* main_page = lv_menu_page_create(menu, "menu");
	cont = lv_menu_cont_create(main_page);
	label = lv_label_create(cont);
	lv_label_set_text(label, "->Running");
	lv_menu_set_load_page_event(menu, cont, sub_1_page);
	/*show running page*/
	setpageCont = cont;

	cont = lv_menu_cont_create(main_page);
	label = lv_label_create(cont);
	lv_label_set_text(label, "->Settings");
	lv_menu_set_load_page_event(menu, cont, sub_2_page);

	//cont = lv_menu_cont_create(main_page);
	//label = lv_label_create(cont);
	//lv_label_set_text(label, "->Item 3");
	////lv_menu_set_load_page_event(menu, cont, sub_3_page);

	lv_menu_set_page(menu, main_page);
	lv_obj_clear_flag(menu, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_clear_flag(sub_1_page, LV_OBJ_FLAG_SCROLLABLE);
	lv_event_send(setpageCont, LV_EVENT_CLICKED, 0);

	//lv_menu_setpage
}
