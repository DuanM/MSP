#include <platform.h>

static pool_t *kbuf_big_pool;
static pool_t *kbuf_small_pool;

bool_t kbuf_init(void)
{
	kbuf_big_pool = (pool_t *)pool_create(KBUF_BIG_NUM, sizeof(kbuf_t) + KBUF_BIG_SIZE);
	DBG_ASSERT(kbuf_big_pool != PLAT_NULL);
	
	kbuf_small_pool = (pool_t *)pool_create(KBUF_SMALL_NUM, sizeof(kbuf_t) + KBUF_SMALL_SIZE);
	DBG_ASSERT(kbuf_small_pool != PLAT_NULL);
	
	return PLAT_TRUE;
}

kbuf_t *kbuf_alloc(uint8_t type)
{
	OSEL_DECL_CRITICAL();
	kbuf_t *kbuf = PLAT_NULL;	

	OSEL_ENTER_CRITICAL();
	
	if (type == KBUF_SMALL_TYPE)
	{	
		kbuf = pool_alloc(kbuf_small_pool);
		if (kbuf)
		{
			kbuf->type = KBUF_SMALL_TYPE;
            kbuf->valid_len = 0;
			kbuf->base = (uint8_t *)kbuf + sizeof(kbuf_t);
			kbuf->offset = kbuf->base;
			kbuf->priv = PLAT_NULL;
            
            //DBG_PRINTF("\r\n-%d\r\n", kbuf_small_pool->blk_rem_num);
		}
	}
	else if (type == KBUF_BIG_TYPE)
	{
		kbuf = pool_alloc(kbuf_big_pool);
		if (kbuf)
		{
			kbuf->type = KBUF_BIG_TYPE;
            kbuf->valid_len = 0;
			kbuf->base = (uint8_t *)kbuf + sizeof(kbuf_t);
			kbuf->offset = kbuf->base;
			kbuf->priv = PLAT_NULL;
            
//			DBG_PRINTF("-%d\r\n", kbuf_big_pool->blk_rem_num);
		}		
	}
    
	OSEL_EXIT_CRITICAL();
	return kbuf;
}

void *kbuf_free(kbuf_t *kbuf)
{
	OSEL_DECL_CRITICAL();
	if (kbuf == PLAT_NULL) return PLAT_NULL;

	OSEL_ENTER_CRITICAL();
	
	if (kbuf->type & KBUF_SMALL_TYPE)
	{		
		pool_free(kbuf_small_pool, kbuf);
        
        //DBG_PRINTF("\r\n+%d\r\n", kbuf_small_pool->blk_rem_num);
	}
	else
	{
		pool_free(kbuf_big_pool, kbuf);
		
//        DBG_PRINTF("+%d\r\n", kbuf_big_pool->blk_rem_num);
	}
    
	OSEL_EXIT_CRITICAL();
	
	return PLAT_NULL;
}

bool_t kbuf_is_empty(uint8_t type)
{
	if (type == KBUF_SMALL_TYPE)
	{
		if (kbuf_small_pool->blk_rem_num == 0) return PLAT_TRUE;
		else return PLAT_FALSE;
	}
	else if(type == KBUF_BIG_TYPE)
	{
		if (kbuf_big_pool->blk_rem_num == 0) return PLAT_TRUE;
		else return PLAT_FALSE;
	}
	else
	{
	  	return PLAT_FALSE;
	}
}

int32_t kbuf_get_num(uint8_t type)
{
	if (type == KBUF_SMALL_TYPE)
	{
		return kbuf_small_pool->blk_rem_num;
	}
	else if(type == KBUF_BIG_TYPE)
	{
		return kbuf_big_pool->blk_rem_num;
	}
	else
	{
	  	return -1;
	}
}
