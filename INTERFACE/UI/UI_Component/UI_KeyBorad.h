#ifndef  __UI_KeyBorad_H_
#define __UI_KeyBorad_H_
#include "../UI_Include.h"
typedef enum 
{
	NumberKey,
	NumberChar,
}UI_keyBorad_Typed;

lv_obj_t* create_custom_keyboard(lv_obj_t* parent, int typed);
lv_obj_t* get_custom_keyboard(void);

#endif // ! __UI_Btn_H_
