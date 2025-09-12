#include "page1.h"
#include "page1_message_id.h"
// #include "page_include.h"
#include "../../UI/page_include.h"
#include "../../UI/Task/task_manager.h"
#include "../../UI/UI_Component/UI_ListBox.h"
#include "../../UI/UI_Parameter/UI_Parameter_Set.h"
#include "../../UI/control/control.h"
#include "../../UI/control/control_Hardware_API.h"
#include "string.h"

uint16_t  Get_Str_Len(const char* str, uint16_t maxlen);
#define COMPARE(a,b) strncmp(a,b,Get_Str_Len(a,50))
static Table_Contain_Property* table_Contain_Property;
lv_timer_t Page1_timer;
lv_obj_t* parent_box;
lv_obj_t* parent_bo2[10];
lv_obj_t* start_btn;
lv_obj_t* Msg_Label;
lv_obj_t* MouseNameTextArea;
char MouseNameTextArea_textSource[50];
uint8_t isSaveUpdata;
int16_t CurrentTask = -1, Interval_ID = -1;
int16_t CurrentCount = -1, Interval_RemainingSec = -1;
uint8_t parent_bo2_index = 0;
//static char msgBuffer[50];
static int touch_counter = 0;
#define Read_Flag(n) (*((volatile) & n))
typedef enum
{
	Flag_required,
	Flag_OKNE,
	Flag_Error
} Control_flag;
struct
{
	Control_flag flag;
	uint16_t fouce_index;
	Task_Parameter_Struct info;
	Task_Parameter_Struct* info_pt;
} Page_Add_Task;

struct
{
	Control_flag flag;
	uint16_t fouce_index;
	Task_Parameter_Struct Set;
	Task_Parameter_Struct* para;
} Page_Save_Task;

struct
{
	Control_flag flag;
	uint16_t fouce_index;
	Task_Parameter_Struct info;
	Task_Parameter_Struct* info_pt;
} Page_del_Task;

// void UI_Page1_Refresh_handle(lv_time_t )

static uint16_t  Get_Str_Len(const char* str, uint16_t maxlen)
{
	int i = 0;
	for (; i < maxlen; i++)
	{
		if (str[i] == 0)
		{
			return (i == 0) ? 0 : i - 1;
		}
	}
	return 0;
}

void Set_table_Cell_Text(lv_obj_t* obj, Task_Parameter_Struct* e)
{
	static int8_t updata_count = -1;
	if (isSaveUpdata == 0)
		return;
	char strs[25];
	Table_Property* item_property = UI_Table_Get_Property(obj);
	if (updata_count > table_Contain_Property->list->size * 2)
	{
		isSaveUpdata = 0;
		updata_count = -1;
	}
	updata_count = *item_property->ID;
	switch (e->mode)
	{
	case Task_VOR:
		if (*item_property->ID == CurrentTask)
			lv_table_set_cell_value_fmt(obj, 0, 1, "%d/%d", CurrentCount, (int)e->VOR.Counter);
		else
			lv_table_set_cell_value_fmt(obj, 0, 1, "%d", (int)e->VOR.Counter);
		sprintf(strs, "%.1f/hz", e->VOR.Freq);
		// sprintf(strs, "%.1f/hz", lv_tick_get() / 1000.0f);
		lv_table_set_cell_value_fmt(obj, 0, 2, "%s", strs);
		sprintf(strs, "%.0f°/s", e->VOR.Vel);
		lv_table_set_cell_value_fmt(obj, 0, 3, "%s", strs);
		break;
	case Task_Continue:
		if (*item_property->ID == CurrentTask)
			lv_table_set_cell_value_fmt(obj, 0, 1, "%ds/%ds", CurrentCount, e->CONT.Sec);
		else
			lv_table_set_cell_value_fmt(obj, 0, 1, "%ds", e->CONT.Sec);
		sprintf(strs, "%.0f°/s", e->CONT.Vel);
		lv_table_set_cell_value_fmt(obj, 0, 2, "%s", strs);
		lv_table_set_cell_value_fmt(obj, 0, 3, " ");
		break;
	case Task_OVAR:
		if (*item_property->ID == CurrentTask)
			lv_table_set_cell_value_fmt(obj, 0, 1, "%d/%d", CurrentCount, (int)e->OVAR.Sec);
		else
			lv_table_set_cell_value_fmt(obj, 0, 1, "%d", (int)e->OVAR.Sec);
		sprintf(strs, "%.1f°", e->OVAR.Inc_Degree);
		lv_table_set_cell_value_fmt(obj, 0, 2, "%s", strs);
		sprintf(strs, "%.0f°/s", e->OVAR.Vel);
		lv_table_set_cell_value_fmt(obj, 0, 3, "%s", strs);
		break;
	}
}

#define SPRINTF sprintf_s
void ui1_(Table_Property* p)
{
	short index = *p->ID;
	if (p->Updata_Source)
	{
		Task_Parameter_Struct* task_info = p->Updata_Source;
		const char mode_name_array[4][6] = { "NULL","VOR", "CONT","OVAR" };
		uint16_t array_index = task_info->mode + 1;
		if (array_index >= 0x04)
		{
			array_index = 0;
		}
		const char* mode_Name = mode_name_array[(array_index)];
		lv_table_set_cell_value_fmt(p->obj, 0, 0, "%s,%d", mode_Name, index);
		Set_table_Cell_Text(p->obj, task_info);
	}
}
//uint8_t Control_btn_communication(uint8_t message[50], uint8_t* cmd)
//{
//	char* CMD = cmd;
//	if (CmdCheck(cmd, CMD_BTN_ADD) == 0)
//	{
//		// 执�?�添�?
//	}
//	if (Control_Add_Task.flag == Flag_required)
//	{
//		uint16_t index = Control_Add_Task.fouce_index;
//		Control_Add_Task.info_pt = Task_Stroage_Insert(Control_Add_Task.info, index);
//		if (Control_Add_Task.info_pt != 0)
//			Control_Add_Task.flag = Flag_OKNE; // 让参数继�?运�??
//		else
//			Control_Add_Task.flag = Flag_Error;
//		return 0;
//	}
//	if (Control_del_Task.flag == Flag_required)
//	{
//		uint16_t index = Control_del_Task.fouce_index;
//		if (Task_Stroage_delByID(index))
//			Control_del_Task.flag = Flag_OKNE; // 让参数继�?运�??
//		else
//			Control_del_Task.flag = Flag_Error;
//		return 0;
//	}
//	if (Control_Save_Task.flag == Flag_required)
//	{
//		uint16_t index = Control_Save_Task.fouce_index;
//		if (Control_Save_Task.para == 0)
//		{
//			Control_Save_Task.flag = Flag_Error;
//		}
//		else
//		{
//			memcpy(Control_Save_Task.para, &Control_Save_Task.Set, sizeof(Task_Parameter_Struct));
//			Control_Save_Task.flag = Flag_OKNE;
//		}
//		return 0;
//	}
//	return 0;
//}

void UI_Task_Btn_ADD_Callback(uint16_t fouces_index, Task_Parameter_Struct* pt)
{
	touch_counter++;
	lv_obj_t* table = UI_Table_Create(table_Contain_Property, ui1_, fouces_index);
	Table_Property* table_Property = UI_Table_Get_Property(table);
	parent_bo2[parent_bo2_index++] = table;
	// 设置好数�?�?
	table_Property->Updata_Source = pt;
	UI_Table_Set_Fouces(table_Contain_Property, fouces_index);
}

void UI_Task_Btn_ADD_Click_Event(lv_event_t* e)
{
	uint32_t handleID;
	// volatile Control_flag *add_flag = &Control_Add_Task.flag;		//避免汇编�?访问reg导致错�??
	uint8_t res = UI_Parameter_Read(&Page_Add_Task.info);
	if (res == 0)
		return;

	int fouces_index = UI_Table_Get_Fouces(table_Contain_Property);
	Page_Add_Task.fouce_index = fouces_index;
	Task_manager_Begin_Req(&handleID);
	Task_manager_Req_add(handleID, fouces_index, Page_Add_Task.info);
	Task_manager_End_release(handleID);
}

void UI_Task_Btn_Move_Callback(uint16_t swap_index1, uint16_t swap_index2)
{
	uint16_t fouces_index = 0;
	UI_Table_Move(table_Contain_Property, swap_index1, swap_index2);
	int task_size = UI_Table_Find_Size(table_Contain_Property);
	if (task_size > swap_index2)
	{
		fouces_index = swap_index2;
	}
	UI_Table_Set_Fouces(table_Contain_Property, fouces_index);

}

void UI_Task_Btn_Move_Click_Event(lv_event_t* e)
{
	uint32_t handleID;
	int fouces_index = UI_Table_Get_Fouces(table_Contain_Property);
	Task_manager_Begin_Req(&handleID);
	Task_manager_Req_Move(handleID, fouces_index, fouces_index + 1);
	Task_manager_End_release(handleID);
}

void UI_Task_Btn_Save_Callback(uint16_t fouces_index, Task_Parameter_Struct* src, Task_Parameter_Struct* traget)
{
	;
}

void UI_Task_Btn_Save_Click_Event(lv_event_t* e)
{
	uint32_t handleID = 0;
	Task_Parameter_Struct src;
	uint8_t res = UI_Parameter_Read(&src);
	int fouces_index = UI_Table_Get_Fouces(table_Contain_Property);
	Task_manager_Begin_Req(&handleID);
	Task_manager_Req_Save(handleID, fouces_index, src);
	Task_manager_End_release(handleID);
}

void UI_Task_Btn_Del_Callback(uint16_t fouces_index)
{
	UI_Table_Clean(table_Contain_Property->list, fouces_index, 0);
}

void UI_Task_table_CLEAN_Callback()
{
	int listsize = table_Contain_Property->list->size;
	while (listsize--)
	{
		UI_Table_Clean(table_Contain_Property->list, 0, 0);
	}
}

void UI_Task_Btn_Del_Click_Event(lv_event_t* e)
{
	static const Task_Parameter_Struct del_templete = { .mode = 0xff };
	int fouces_index = UI_Table_Get_Fouces(table_Contain_Property);
	if (fouces_index < 0)
		return;
	Table_Property* item_property = UI_Table_Find_Obj_User_Data(table_Contain_Property, fouces_index);
	item_property->Updata_Source = (void*)&del_templete;
	LV_LOG_USER("DEL %d", fouces_index);
	uint32_t handleID;
	Task_manager_Begin_Req(&handleID);
	Task_manager_Req_Del(handleID, fouces_index);
	Task_manager_End_release(handleID);
}

void UI_Task_Btn_Init(lv_obj_t* parent)
{
	const char string[][5] = { "add", "move", "save", "del" };
	lv_obj_t* btn[4];
	for (int i = 0; i < 4; i++)
	{
		btn[i] = lv_btn_create(parent);
		lv_obj_set_size(btn[i], 70, 40);
		lv_obj_t* label = lv_label_create(btn[i]); /*Add a label to the button*/
		lv_label_set_text(label, string[i]);	   /*Set the labels text*/
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
		lv_obj_align(btn[i], LV_ALIGN_DEFAULT, i * 80, 0);
	}
	lv_obj_add_event_cb(btn[0], UI_Task_Btn_ADD_Click_Event, LV_EVENT_CLICKED, 0);
	lv_obj_add_event_cb(btn[1], UI_Task_Btn_Move_Click_Event, LV_EVENT_CLICKED, 0);
	lv_obj_add_event_cb(btn[2], UI_Task_Btn_Save_Click_Event, LV_EVENT_CLICKED, 0);
	lv_obj_add_event_cb(btn[3], UI_Task_Btn_Del_Click_Event, LV_EVENT_CLICKED, 0);
	// parent_bo2 = lv_label_create(parent);
}
char string[50];
void UI_Task_Msg_Init(lv_obj_t* parent)
{
	Msg_Label = lv_label_create(parent);
	lv_obj_align_to(Msg_Label, start_btn, LV_ALIGN_OUT_BOTTOM_LEFT, -200, -20);
	lv_label_set_text(Msg_Label, string);

}

static uint8_t StartBtnFlag;

void UI_Start_Btn_Clicked_Handle(lv_event_t* e)
{
	if (e->code == LV_EVENT_CLICKED)
	{
		Message_Center_Send_prinft("Ctrl", 0,
			0,
			"ReqShift");
	}
}

// void UI_Start_Btn_Refresh_Handle(lv_event_t* e)
// {
// 	if (e->code == LV_EVENT_CLICKED)
// 	{
// 		StartBtnFlag = 1;
// 	}
// }

void UI_Start_Btn_Init(lv_obj_t* parent)
{
	lv_obj_t* btn = lv_btn_create(parent);
	start_btn = btn;
	lv_obj_t* label = lv_label_create(btn); /*Add a label to the button*/
	lv_label_set_text(label, "start");		/*Set the labels text*/
	lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_obj_add_event_cb(btn, UI_Start_Btn_Clicked_Handle, LV_EVENT_CLICKED, 0);
}

uint8_t UI_Start_Btn_Get_CMD(void)
{
	uint8_t flag = StartBtnFlag;
	StartBtnFlag = 0;
	return flag;
}

char Page1_meassege[50];
uint8_t Page1_meassege_flag;
char* UI_Page1_Get_Msg_Array(uint16_t* len)
{
	if (len != 0)
		*len = sizeof(Page1_meassege);
	if (Page1_meassege_flag == 0)
	{

		Page1_meassege[0] = 0;
		return Page1_meassege;
	}
	else
		return 0;
}

void UI_Page1_Set_Msg_Finish(void)
{
	Page1_meassege_flag = 1;
}

struct
{
	enum {
		cmd_none,
		cmd_add,
		cmd_move,
		cmd_del,
		cmd_save,
		cmd_clean
	}cmd_typed;
	Task_Parameter_Struct* task;
	uint16_t task_id;
	uint16_t task_id2;
	uint8_t isvaild;
	uint16_t error_counter;
}task_msg;

void UI_Page1_Send_ADD_Cmd(uint8_t* msg, uint16_t msg_size,
	uint8_t* src, uint16_t SrcSize)
{
	//if (task_msg.isvaild)
	//	return;
	if (Msg_COMPARE("ADD", msg))
	{
		int index = 0;
		if (sscanf(msg, "ADD %d", &index) == 1 &&
			SrcSize == sizeof(void*))
		{
			task_msg.isvaild = 1;
			task_msg.cmd_typed = cmd_add;
			task_msg.task_id = index;
			task_msg.task = (Task_Parameter_Struct*)src;
		}
	}
	if (Msg_COMPARE("DEL", msg))
	{
		int index = 0;
		if (sscanf(msg, "DEL %d", &index) == 1)
		{
			task_msg.cmd_typed = cmd_del;
			task_msg.task_id = index;
			task_msg.isvaild = 1;
		}
	}
	if (Msg_COMPARE("SETZERO", msg))
	{
		task_msg.cmd_typed = cmd_clean;
		task_msg.isvaild = 1;
	}
	if (Msg_COMPARE("Save", msg))
	{
		int index = 0;
		if (sscanf(msg, "Save %d", &index) == 1 &&
			SrcSize == sizeof(Task_Parameter_Struct*))
		{

			/*task_msg.cmd_typed = cmd_save;
			task_msg.task = src;
			task_msg.task_id = index;
			task_msg.isvaild = 1;*/
		}
	}
	if (Msg_COMPARE("Move", msg))
	{
		int id = 0, id2 = 0;
		if (sscanf(msg, "Move %d %d", &id, &id2) == 2)
		{
			task_msg.cmd_typed = cmd_move;
			task_msg.task_id = id;
			task_msg.task_id2 = id2;
			task_msg.isvaild = 1;
		}
	}
	isSaveUpdata = 1;

}

MsgReadReturn UI_Page1_Send_MouseName(uint8_t* msg, uint16_t msg_size,
	uint8_t* src, uint16_t SrcSize)
{
	if (Msg_COMPARE("MouseName?", msg))
	{
		strncpy(src, MouseNameTextArea_textSource, SrcSize);
		return msg_match;
	}
	return msg_nomatch;
}

void UI_Page1_Get_Souce_Updata()
{
	if (task_msg.isvaild)
	{
		uint8_t cmdTpyed = task_msg.cmd_typed;
		if (cmdTpyed == cmd_add)
		{
			UI_Task_Btn_ADD_Callback(task_msg.task_id,
				task_msg.task);
		}
		if (cmdTpyed == cmd_move)
		{
			UI_Task_Btn_Move_Callback(task_msg.task_id,
				task_msg.task_id2);
		}
		if (cmdTpyed == cmd_del)
		{
			UI_Task_Btn_Del_Callback(task_msg.task_id);
		}
		if (cmdTpyed == cmd_clean)
		{
			UI_Task_table_CLEAN_Callback();
		}
		task_msg.isvaild = 0;
	}
	//	uint16_t funid = 0;
	//	void* pt = 0;
	//	uint8_t flag = Message_Center_Get(
	//		"task", funid,
	//		&pt,
	//		msgBuffer, sizeof(msgBuffer));
	//	if (flag == 0)
	//	{
	//		if (COMPARE("ADD", msgBuffer) == 0)
	//		{
	//			if (pt == 0)
	//				return;
	//			Task_Parameter_Struct* task_pt = pt;
	//			int add_index = 0;
	//			if (sscanf(msgBuffer, "ADD %d", &add_index) == 1)
	//			{
	//				UI_Task_Btn_ADD_Callback(add_index, task_pt);
	//			};
	//		}
	//		if (COMPARE("DEL", msgBuffer) == 0)
	//		{
	//			int del_index = 0;
	//			if (sscanf(msgBuffer, "DEL %d", &del_index) == 1)
	//			{
	//				UI_Task_Btn_Del_Callback(del_index);
	//			};
	//		}
	//		if (COMPARE("Save", msgBuffer) == 0)
	//		{
	//#if 0
	//			int _index = 0;
	//			if (sscanf(msgBuffer, "Save %d", &del_index) == 1)
	//			{
	//				//UI_Task_Btn_Save_Callback(del_index);
	//			};
	//#endif
	//		}
	//		if (COMPARE("Move", msgBuffer) == 0)
	//		{
	//			int swap1 = 0, swap2;
	//			if (sscanf(msgBuffer, "Move %d %d", &swap1, &swap2) == 2)
	//			{
	//				UI_Task_Btn_Move_Callback(swap1, swap2);
	//			};
	//		}
	//		Message_Center_Clean_Flag("task", funid);
	//
	//	}
}

void UI_Page1_Btn_Refulsh()
{
	const int mallocSize = 40;
	char* str = message_malloc(mallocSize);
	if (str == 0)
		return;

	memset(str, 0, mallocSize);
	Message_Center_Read_prinft("Ctrl", str, mallocSize, "run_State?");
	if (Msg_COMPARE("isStart", str))
	{
		lv_obj_t* label = lv_obj_get_child(start_btn, 0);
		// start:ON
		lv_label_set_text_fmt(label, "%s", "STOP");
	}
	if (Msg_COMPARE("isStop", str))
	{
		lv_obj_t* label = lv_obj_get_child(start_btn, 0);
		// start:ON
		lv_label_set_text_fmt(label, "%s", "START");
	}
	message_free(str);
}

void UI_Page1_Msg_Refulsh()
{
	const int mallocSize = 40;
	char* str = message_malloc(mallocSize);
	if (str == 0)
		return;
	memset(str, 0, mallocSize);
	if (Message_Center_Read_prinft("Ctrl", str, mallocSize, "Msg?") == 0)
	{
		lv_label_set_text(Msg_Label, str);
		lv_obj_invalidate(Msg_Label);
	}
	free(str);
	//const char *newline = "\r\n";
	//const int mallocSize = 40;
	//char* str = message_malloc(mallocSize);
	//if (str == 0)
	//	return;
	//memset(str, 0, mallocSize);
	//if (Message_Center_Read_prinft("Ctrl", str, mallocSize, "Msg?") == 0)
	//{
	//	lv_textarea_add_text(Msg_Label, str);
	//	lv_textarea_add_text(Msg_Label, newline);
	//	/*	lv_label_set_text(Msg_Label, str);
	//		lv_obj_invalidate(Msg_Label);*/
	//}
	//free(str);
}


void UI_Page1_Respone_Updata_Msg(uint8_t flag, uint16_t ID)
{
	if (flag == 0 && ID == ID_Bl_ctrl)
	{

	}
}

void UI_Page1_Timer_UIupdata_handle(lv_timer_t* t)
{
	UI_Page1_Btn_Refulsh();
	UI_Page1_Msg_Refulsh();
}

void UI_Page1_Timer_handle(lv_timer_t* t)
{
	UI_Page1_Get_Souce_Updata();
}

static void ta_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* ta = lv_event_get_target(e);
	lv_obj_t* kb = lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		strncpy(MouseNameTextArea_textSource,
			lv_textarea_get_text(e->current_target),
			50);
	}

	if (code == LV_EVENT_DEFOCUSED) {
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		//lv_obj_invalidate(kb);
	}
}

static void keyboardHidden(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CANCEL) {
		lv_obj_add_flag(e->current_target, LV_OBJ_FLAG_HIDDEN);
	}
}


void UI_mouse_Name_textInput(lv_obj_t* parent)
{
	/*Create a keyboard to use it with an of the text areas*/
	lv_obj_t* kb = lv_keyboard_create(lv_scr_act());
	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

	/*Create a text area. The keyboard will write here*/
	lv_obj_t* ta;
	ta = lv_textarea_create(parent);
	MouseNameTextArea = ta;
	lv_obj_align(ta, LV_ALIGN_TOP_LEFT, -0, 120);
	lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
	lv_textarea_set_placeholder_text(ta, "enter mouse name");
	lv_textarea_set_accepted_chars(ta, "_1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()-=+");
	lv_textarea_set_max_length(ta, 10);
	lv_obj_add_event_cb(kb, keyboardHidden, LV_EVENT_CANCEL, 0);
	lv_obj_set_size(ta, 240, 45);
}

void Page1_init(lv_obj_t* parent)
{
	parent_box = parent;
	table_Contain_Property = UI_ListBox_Create(parent);
	//user expirement para set
	Mode_init(parent);
	// UI_Table_Create(list5, 0);
	/*btn*/
	UI_Task_Btn_Init(parent);   //task btn include -add-save-move-del

	// task begin btn
	UI_Start_Btn_Init(parent);
	UI_Task_Msg_Init(parent);

	// meassage
	Message_Center_Add_Send_CB("task", UI_Page1_Send_ADD_Cmd);
	Message_Center_Add_Read_CB("page1", UI_Page1_Send_MouseName);
	// mouse ID inter
	UI_mouse_Name_textInput(parent);

	lv_timer_create(UI_Page1_Timer_handle, 5, 0);
	lv_timer_create(UI_Page1_Timer_UIupdata_handle, 100, 0);

}
