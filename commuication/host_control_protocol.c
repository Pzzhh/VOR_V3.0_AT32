#include "host_control_protocol.h"

#include <stdio.h>
#include <string.h>

#include "../INTERFACE/UI/control/control.h"
#include "../INTERFACE/UI/other/meassage_center.h"
#include "../INTERFACE/UI/other/system_function.h"
#include "../INTERFACE/UI/Task/task_stroage.h"

#define HOST_CONTROL_FRAME_SIZE 160
#define HOST_CONTROL_REPLY_SIZE 128
#define HOST_CONTROL_QUEUE_SIZE 8

static HostControl_SendCallback host_send_cb;
static char host_rx_frame[HOST_CONTROL_FRAME_SIZE];
static uint16_t host_rx_length;
static char host_frame_queue[HOST_CONTROL_QUEUE_SIZE][HOST_CONTROL_FRAME_SIZE];
static volatile uint8_t host_queue_head;
static volatile uint8_t host_queue_tail;
static uint8_t host_receiving;
static uint8_t host_overflow;
static Task_Parameter_Struct replace_tasks[Task_ARY_MAX];
static uint16_t replace_count;
static uint8_t replace_active;

extern Task_control_info control_info;
extern uint8_t VOR_exam_Para_Check(Task_Parameter_Struct* e);
extern uint8_t Cont_exam_Para_Check(Task_Parameter_Struct* e);
extern uint8_t Ovar_exam_Para_Check(Task_Parameter_Struct* e);
extern uint8_t VHIT_exam_Para_Check(Task_Parameter_Struct* e);
extern uint8_t TC_exam_Para_Check(Task_Parameter_Struct* e);

static void HostControl_SendReply(const char* reply)
{
    uint8_t frame[HOST_CONTROL_REPLY_SIZE + 2];
    size_t reply_len = reply == 0 ? 0 : strlen(reply);

    if (host_send_cb == 0)
        return;
    if (reply_len > HOST_CONTROL_REPLY_SIZE)
        reply_len = HOST_CONTROL_REPLY_SIZE;
    frame[0] = '(';
    if (reply_len != 0)
        memcpy(&frame[1], reply, reply_len);
    frame[reply_len + 1] = ')';
    host_send_cb(frame, (uint16_t)(reply_len + 2));
}

static uint8_t HostControl_ParseTask(const char* text, Task_Parameter_Struct* task)
{
    int ex_mode = 0;
    int direction = 0;

    memset(task, 0, sizeof(*task));
    if (strncmp(text, "vor,", 4) == 0)
    {
        task->mode = Task_VOR;
        if (sscanf(text, "vor,%u,%f,%f,%d", &task->VOR.Counter,
                   &task->VOR.Vel, &task->VOR.Freq, &ex_mode) < 3)
            return 0;
        if (ex_mode < Ex_VOR || ex_mode > Ex_BOTH_R)
            return 0;
        task->VOR.ExMode = (Task_Parameter_VOR_Exmode)ex_mode;
        return VOR_exam_Para_Check(task);
    }
    if (strncmp(text, "cont,", 5) == 0)
    {
        task->mode = Task_Continue;
        if (sscanf(text, "cont,%d,%f,%f", &task->CONT.Sec,
                   &task->CONT.Vel, &task->CONT.okr_Vel) < 2)
            return 0;
        return Cont_exam_Para_Check(task);
    }
    if (strncmp(text, "ovar,", 5) == 0)
    {
        task->mode = Task_OVAR;
        if (sscanf(text, "ovar,%d,%f,%f", &task->OVAR.Sec,
                   &task->OVAR.Vel, &task->OVAR.Inc_Degree) != 3)
            return 0;
        return Ovar_exam_Para_Check(task);
    }
    if (strncmp(text, "vhit,", 5) == 0)
    {
        task->mode = Task_VHIT;
        if (sscanf(text, "vhit,%u,%d", &task->VHIT.Counter, &direction) != 2 ||
            (direction != 0 && direction != 1))
            return 0;
        task->VHIT.IsCW = (uint8_t)direction;
        return VHIT_exam_Para_Check(task);
    }
    if (strncmp(text, "tc,", 3) == 0)
    {
        task->mode = Task_TC;
        if (sscanf(text, "tc,%u,%f", &task->TC.Counter, &task->TC.Vel) != 2)
            return 0;
        return TC_exam_Para_Check(task);
    }
    return 0;
}

static void HostControl_FormatTask(char* reply, uint16_t reply_size,
                                   uint16_t index, const Task_Parameter_Struct* task)
{
    switch (task->mode)
    {
    case Task_VOR:
        snprintf(reply, reply_size, "TASK,%u,vor,%u,%.3f,%.3f,%d", index,
                 task->VOR.Counter, task->VOR.Vel, task->VOR.Freq, task->VOR.ExMode);
        break;
    case Task_Continue:
        snprintf(reply, reply_size, "TASK,%u,cont,%d,%.3f,%.3f", index,
                 task->CONT.Sec, task->CONT.Vel, task->CONT.okr_Vel);
        break;
    case Task_OVAR:
        snprintf(reply, reply_size, "TASK,%u,ovar,%d,%.3f,%.3f", index,
                 task->OVAR.Sec, task->OVAR.Vel, task->OVAR.Inc_Degree);
        break;
    case Task_VHIT:
        snprintf(reply, reply_size, "TASK,%u,vhit,%u,%u", index,
                 task->VHIT.Counter, task->VHIT.IsCW);
        break;
    case Task_TC:
        snprintf(reply, reply_size, "TASK,%u,tc,%u,%.3f", index,
                 task->TC.Counter, task->TC.Vel);
        break;
    default:
        snprintf(reply, reply_size, "errorMsg");
        break;
    }
}

static uint8_t HostControl_TaskRead(uint8_t* msg, uint16_t msg_size,
                                    uint8_t* src, uint16_t src_size)
{
    Task_Parameter_Struct task;
    Task_Parameter_Struct old_tasks[Task_ARY_MAX];
    uint16_t old_count;
    uint16_t i;
    (void)msg_size;

    if (msg == 0 || src == 0 || src_size == 0)
        return 1;
    if (strcmp((char*)msg, "delAll") == 0)
    {
        if (control_info.State_Bit.IsRunning || control_info.State_Bit.isRun)
        {
            snprintf((char*)src, src_size, "ERR,STATE");
            return 0;
        }
        Task_Stroage_delAll();
        snprintf((char*)src, src_size, "delAll,OK");
        return 0;
    }
    if (strcmp((char*)msg, "ReplaceBegin") == 0)
    {
        if (control_info.State_Bit.IsRunning || control_info.State_Bit.isRun)
            snprintf((char*)src, src_size, "ERR,STATE");
        else
        {
            replace_count = 0;
            replace_active = 1;
            snprintf((char*)src, src_size, "ReplaceBegin,OK");
        }
        return 0;
    }
    if (strcmp((char*)msg, "ReplaceAbort") == 0)
    {
        replace_count = 0;
        replace_active = 0;
        snprintf((char*)src, src_size, "ReplaceAbort,OK");
        return 0;
    }
    if (strcmp((char*)msg, "ReplaceCommit") == 0)
    {
        if (!replace_active || control_info.State_Bit.IsRunning || control_info.State_Bit.isRun)
        {
            snprintf((char*)src, src_size, "ERR,STATE");
            return 0;
        }
        old_count = Task_Stroage_GetArray(old_tasks, Task_ARY_MAX);
        Task_Stroage_delAll();
        for (i = 0; i < replace_count; ++i)
        {
            if (Task_Stroage_Insert(replace_tasks[i], 0xffff) == 0)
                break;
        }
        if (i != replace_count)
        {
            Task_Stroage_delAll();
            for (i = 0; i < old_count; ++i)
                Task_Stroage_Insert(old_tasks[i], 0xffff);
            snprintf((char*)src, src_size, "ERR,MEMORY");
        }
        else
        {
            snprintf((char*)src, src_size, "ReplaceCommit,OK");
        }
        replace_count = 0;
        replace_active = 0;
        return 0;
    }
    if (strncmp((char*)msg, "addtask,", 8) == 0 &&
        HostControl_ParseTask((char*)msg + 8, &task))
    {
        if (control_info.State_Bit.IsRunning || control_info.State_Bit.isRun)
        {
            snprintf((char*)src, src_size, "ERR,STATE");
            return 0;
        }
        if (replace_active)
        {
            if (replace_count >= Task_ARY_MAX)
            {
                snprintf((char*)src, src_size, "ERR,FULL");
                return 0;
            }
            replace_tasks[replace_count++] = task;
        }
        else if (Task_Stroage_GetSize() >= Task_ARY_MAX)
        {
            snprintf((char*)src, src_size, "ERR,FULL");
            return 0;
        }
        else if (Task_Stroage_Insert(task, 0xffff) == 0)
        {
            snprintf((char*)src, src_size, "ERR,MEMORY");
            return 0;
        }
        snprintf((char*)src, src_size, "addtask,OK");
        return 0;
    }
    snprintf((char*)src, src_size, "ERR,PARAM");
    return 0;
}

static uint8_t HostControl_HandleCtrlWrite(const char* command, char* reply, uint16_t reply_size)
{
    if (!control_info.State2_Bit.H_init)
    {
        snprintf(reply, reply_size, "ERR,NOT_INIT");
        return 1;
    }
    if (strcmp(command, "ReqStrat") == 0)
    {
        if (control_info.State_Bit.IsRunning || control_info.State_Bit.isRun)
            snprintf(reply, reply_size, "ERR,STATE");
        else if (Task_Stroage_GetSize() == 0)
            snprintf(reply, reply_size, "ERR,EMPTY");
        else if (Message_Center_Send_prinft("Ctrl", 0, 0, "%s", command) == 0)
            snprintf(reply, reply_size, "W OK");
        return 1;
    }
    if (strcmp(command, "ReqPause") == 0)
    {
        if (!control_info.State_Bit.IsRunning || control_info.State_Bit.isPause)
            snprintf(reply, reply_size, "ERR,STATE");
        else if (Message_Center_Send_prinft("Ctrl", 0, 0, "%s", command) == 0)
            snprintf(reply, reply_size, "W OK");
        return 1;
    }
    if (strcmp(command, "ReqResume") == 0)
    {
        if (!control_info.State_Bit.IsRunning || !control_info.State_Bit.isPause)
            snprintf(reply, reply_size, "ERR,STATE");
        else if (Message_Center_Send_prinft("Ctrl", 0, 0, "%s", command) == 0)
            snprintf(reply, reply_size, "W OK");
        return 1;
    }
    if (strcmp(command, "ReqStop") == 0)
    {
        if (!control_info.State_Bit.IsRunning && !control_info.State_Bit.isRun)
            snprintf(reply, reply_size, "W OK,ALREADY_STOPPED");
        else if (Message_Center_Send_prinft("Ctrl", 0, 0, "%s", command) == 0)
            snprintf(reply, reply_size, "W OK");
        return 1;
    }
    return 0;
}

static uint8_t HostControl_HandleCtrlRead(const char* command, char* reply, uint16_t reply_size)
{
    int index;
    Task_Parameter_Struct task;

    if (strcmp(command, "GetTaskCount") == 0)
    {
        snprintf(reply, reply_size, "TASKCOUNT,%d", Task_Stroage_GetSize());
        return 1;
    }
    if (sscanf(command, "GetTask,%d", &index) == 1)
    {
        if (index < 0 || !Task_Stroage_Get(&task, (short)index))
            snprintf(reply, reply_size, "errorMsg");
        else
            HostControl_FormatTask(reply, reply_size, (uint16_t)index, &task);
        return 1;
    }
    if (strcmp(command, "GetMsg") == 0)
    {
        if (control_info.message[0] == 0)
            reply[0] = 0;
        else
            snprintf(reply, reply_size, "%s", control_info.message);
        return 1;
    }
    if (strcmp(command, "GetError") == 0)
    {
        if (control_info.Error_Bit.flag == 0 &&
            control_info.Rk3588_Flag.LerrorCode == 0 &&
            control_info.Rk3588_Flag.RerrorCode == 0)
        {
            reply[0] = 0;
        }
        else
        {
            snprintf(reply, reply_size, "ERROR,%u,%u,%u,%u,%u",
                     control_info.Error_Bit.flag,
                     control_info.Rk3588_Flag.WifiMsgFlag,
                     control_info.Rk3588_Flag.LerrorCode,
                     control_info.Rk3588_Flag.RerrorCode,
                     (uint16_t)(control_info.currentCount + 1));
        }
        return 1;
    }
    return 0;
}

static void HostControl_Dispatch(char* frame)
{
    char* port;
    char* direction;
    char* command;
    char* separator;
    char reply[HOST_CONTROL_REPLY_SIZE + 1] = "errorMsg";

    if (frame[0] != '$')
    {
        HostControl_SendReply(reply);
        return;
    }
    port = frame + 1;
    separator = strchr(port, '$');
    if (separator == 0)
    {
        HostControl_SendReply(reply);
        return;
    }
    *separator = 0;
    direction = separator + 1;
    separator = strchr(direction, '$');
    if (separator == 0)
    {
        HostControl_SendReply(reply);
        return;
    }
    *separator = 0;
    command = separator + 1;

    if (strcmp(direction, "W") == 0)
    {
        if (strcmp(port, "Ctrl") == 0 &&
            HostControl_HandleCtrlWrite(command, reply, sizeof(reply)))
        {
            /* handled with state validation */
        }
        else if (Message_Center_Send_prinft(port, 0, 0, "%s", command) == 0)
            strcpy(reply, "W OK");
    }
    else if (strcmp(direction, "R") == 0)
    {
        if (strcmp(port, "BLctrl") == 0)
        {
            HostControl_TaskRead((uint8_t*)command, (uint16_t)strlen(command),
                                 (uint8_t*)reply, sizeof(reply));
        }
        else if (strcmp(port, "Ctrl") == 0 &&
            HostControl_HandleCtrlRead(command, reply, sizeof(reply)))
        {
            /* handled locally */
        }
        else
        {
            Message_Center_Read_prinft(port, reply, sizeof(reply), "%s", command);
        }
    }
    HostControl_SendReply(reply);
}

void HostControl_Init(HostControl_SendCallback send_cb)
{
    host_send_cb = send_cb;
    host_rx_length = 0;
    host_queue_head = 0;
    host_queue_tail = 0;
    host_receiving = 0;
    host_overflow = 0;
    replace_count = 0;
    replace_active = 0;
    memset(host_rx_frame, 0, sizeof(host_rx_frame));
}

void HostControl_ReceiveByte(uint8_t data)
{
    uint8_t next_tail;

    if (data == '(')
    {
        host_rx_length = 0;
        host_receiving = 1;
        host_overflow = 0;
        memset(host_rx_frame, 0, sizeof(host_rx_frame));
        return;
    }
    if (data == ')')
    {
        if (host_receiving)
        {
            if (host_overflow)
                strcpy(host_rx_frame, "__FRAME_TOO_LONG__");
            else
                host_rx_frame[host_rx_length] = 0;

            next_tail = (uint8_t)((host_queue_tail + 1) % HOST_CONTROL_QUEUE_SIZE);
            if (next_tail != host_queue_head)
            {
                memcpy(host_frame_queue[host_queue_tail], host_rx_frame,
                       sizeof(host_rx_frame));
                host_queue_tail = next_tail;
            }
        }
        host_receiving = 0;
        return;
    }
    if (host_receiving)
    {
        if (host_rx_length < sizeof(host_rx_frame) - 1)
            host_rx_frame[host_rx_length++] = (char)data;
        else
            host_overflow = 1;
    }
}

void HostControl_Process(void)
{
    char frame[HOST_CONTROL_FRAME_SIZE];
    if (host_queue_head == host_queue_tail)
        return;
    memcpy(frame, host_frame_queue[host_queue_head], sizeof(frame));
    host_queue_head = (uint8_t)((host_queue_head + 1) % HOST_CONTROL_QUEUE_SIZE);
    if (strcmp(frame, "__FRAME_TOO_LONG__") == 0)
    {
        HostControl_SendReply("ERR,FRAME_TOO_LONG");
        return;
    }
    HostControl_Dispatch(frame);
}
