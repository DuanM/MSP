#include <ucos_ii.h>
#include <platform.h>

#define STK_MEM_WORD_SIZE	(OSEL_TASK_STACK_SIZE_MAX/sizeof(OS_STK))

/*任务管理池*/
static pool_t *task_pool;

static fpv_t osel_task_idle_hook[OS_MAX_TASKS];

bool_t osel_task_init(void)
{
	task_pool = pool_create(OSEL_TASK_MAX,	sizeof(osel_task_t)+OSEL_TASK_STACK_SIZE_MAX);
	if (task_pool == PLAT_NULL) 
	{
		return PLAT_FALSE;
	}
	
	for(uint8_t index=0; index<OS_MAX_TASKS; index++)
	{
		osel_task_idle_hook[index] = PLAT_NULL;
	}

	return PLAT_TRUE;
}

osel_task_t *osel_task_create(OSEL_TASK_RETURN_TYPE (*taskcode)(OSEL_TASK_PARAM_TYPE),
						OSEL_TASK_PARAM_TYPE param,
						uint32_t stack_depth,
						uint16_t prio
						)
{
    OSEL_DECL_CRITICAL();
    
    OSEL_ENTER_CRITICAL();
	osel_task_t *task;	
	
	if (taskcode == PLAT_NULL)
	{
		return PLAT_NULL;
	}
	// alloc task pool
	task = (osel_task_t *)pool_alloc(task_pool);

	if (task == PLAT_NULL)
	{
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;
	}
	
	/* Allocate stack space */
	if (stack_depth%sizeof(OS_STK) == 0 && STK_MEM_WORD_SIZE >= stack_depth)
	{
		task->stack = (uint8_t *)task+sizeof(osel_task_t);
	}
	else
	{
		pool_free(task_pool, (void *)task);
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;	
	}
	
	task->prio = prio;	
    task->stack_size = stack_depth*sizeof(OS_STK);

	/* Create task */
	if (OSTaskCreateExt(taskcode, 
					   param, 
					   (OS_STK *)task->stack + stack_depth - 1, 
					   (uint8_t)prio,
					   (uint16_t)prio,
					   (OS_STK *)task->stack,
					   stack_depth,
					   NULL,
					   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR) == OS_ERR_NONE)
    {
    	task->handle = (uintptr_t)prio;
        OSEL_EXIT_CRITICAL();
        return task;
    }
	else
	{
		pool_free(task_pool, (void *)task);
        OSEL_EXIT_CRITICAL();
		return PLAT_NULL;
	}
}

bool_t osel_task_delete(osel_task_t *task)
{
    OSEL_DECL_CRITICAL();
    
    OSEL_ENTER_CRITICAL();
	if (task != PLAT_NULL)
	{
        if (pool_free(task_pool, (void *)task) == PLAT_FALSE) 
        {
            OSEL_EXIT_CRITICAL();
            return PLAT_FALSE;
        }			
		
        if (OSTaskDel((uint8_t)(task->handle)) == OS_ERR_NONE)
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_TRUE;
		}
		else
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_FALSE;
		}
	}
	else
	{
        OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}	
}

bool_t osel_task_suspend(osel_task_t *task)
{
	return (OSTaskSuspend((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_resume(osel_task_t *task)
{
	return (OSTaskResume((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_query(osel_task_t *task)
{
	OS_TCB  tcb;
		
	if (task != PLAT_NULL)
	{
		OSTaskQuery(OS_PRIO_SELF, &tcb);

		task->handle = tcb.OSTCBPrio;
		task->prio = tcb.OSTCBPrio;
		task->stack = tcb.OSTCBStkBottom + tcb.OSTCBStkSize;
		task->stack_size = tcb.OSTCBStkSize*sizeof(OS_STK);

		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

void osel_task_idle_hook_reg(uint8_t index, fpv_t func)
{
	if (index<OS_MAX_TASKS)
	{
		osel_task_idle_hook[index] = func;
	}
}

void osel_idle_task_hook(void)
{
	for (uint8_t index=0; index<OS_MAX_TASKS; index++)
	{
		if (osel_task_idle_hook[index])
		{
			osel_task_idle_hook[index]();
		}
	}	
}
