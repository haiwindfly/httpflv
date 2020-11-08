#include <buffer.h>
Buffer*   	buffer_queue;
oal_int8 buffer_init(Buffer *buf,oal_uint16 u_size,oal_uint32 num)
{
   if(buf == oal_ptr_null)
   {
       return INERTNER_ERROR;
   }
   buf->head = (oal_uint8 *)calloc(num,u_size*sizeof(oal_uint8));
   if(buf->head == oal_ptr_null)
   {
       return INERTNER_ERROR;
   }
   buf->uint_num  = num;
   buf->unit_size = u_size;
   buf->end       = buf->head + u_size*num;
   buf->in        = buf->out = buf->head;
   buf->is_on     = true;
   return BUFFER_INIT_SUCCESS;
}
oal_int8 buffer_put(Buffer *buf,oal_uint8 *addr,oal_uint16 real_size)
{
#ifdef BUFFER_CHECK_SAFETY
    if(!buf)
    {
        return INERTNER_ERROR;
    }
    if(real_size != buf->unit_size)
    {
        return INERTNER_ERROR;
    }
#endif
    if((buf->in + buf->unit_size) == buf->out || (buf->out == buf->head && (buf->in + buf->unit_size) == buf->end))
    {
        return BUFFER_FULL;
    }
    memcpy(buf->in,addr,buf->unit_size);
    buf->in = (buf->in + buf->unit_size) == buf->end ? buf->head : (buf->in + buf->unit_size);
    return BUFFER_PUT_SUCCESS;
}
oal_int8 buffer_get(Buffer *buf,oal_uint8 *addr)
{
#ifdef BUFFER_CHECK_SAFETY
    if(!buf)
    {
        return INERTNER_ERROR;
    }
#endif
    if(buf->out == buf->in)
    {
        return BUFFER_EMPTY;
    }
    memcpy(addr,buf->out,buf->unit_size);   
    buf->out = (buf->out + buf->unit_size) == buf->end ?  buf->head : (buf->out + buf->unit_size);
    return BUFFER_GET_SUCCESS;
}
oal_void  buffer_delete(Buffer *buf)
{
    if(!buf)
    {
        return ;
    }
    if(!buf->head)
    {
        return ;
    }
    free(buf->head);
    return;
}