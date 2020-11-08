#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <oal.h>
#define   BUFFER_GET_SUCCESS        0
#define   BUFFER_PUT_SUCCESS        0
#define   BUFFER_INIT_SUCCESS       0
#define   BUFFER_FULL              -1
#define   BUFFER_EMPTY             -1
#define   INERTNER_ERROR           -2
//#define   BUFFER_CHECK_SAFETY
typedef struct buffer
{
    oal_bool    is_on;
    oal_uint8   *head;
    oal_uint8   *end;
    oal_uint8   *in;
    oal_uint8   *out;
    oal_uint16  unit_size;
    oal_uint32  uint_num;
}Buffer;
oal_int8  buffer_init(Buffer *buf,oal_uint16 u_size,oal_uint32 num);
oal_int8  buffer_put(Buffer *buf,oal_uint8 *addr,oal_uint16 real_size);
oal_int8  buffer_get(Buffer *buf,oal_uint8 *addr);
oal_void  buffer_delete(Buffer *buf);

extern Buffer*  buffer_queue;
#endif 