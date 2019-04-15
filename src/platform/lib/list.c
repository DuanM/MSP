#include <platform.h>

//�������Բ��ͷ
void list_init(list_t *head)
{
	head->next = head;
	head->prev = head;
}
/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal dlist manipulation where we know
 * the prev/next entries already!
 * prev -> new_prev -> next 
 */
static inline void __list_add(list_t *new_entry,
			      				list_t *prev,
			      				list_t *next)
{
	next->prev = new_entry;
	new_entry->next = next;
	new_entry->prev = prev;
	prev->next = new_entry;
}

/*
 * Delete a dlist entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal dlist manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(list_t *prev, list_t *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_get_head	-	return the first entry in a list
 *
 * @param  head:	the head for your list.
 *
 * @return  pointer to first list entry, NULL on list empty
 */    
list_t * list_get_head(list_t *head)
{
	return ((list_empty(head))?(list_t *)NULL:((head)->next));
}

//new_entry ���� pos �������
//pos -> new_entry -> ((pos)->next)
void list_front_put(list_t *new_entry, list_t *pos)
{
	__list_add(new_entry, (pos), (pos)->next);
}

//new_entry ���� POS ����ǰ��
//((pos)->prev) -> new_entry -> (pos)
void list_behind_put(list_t *new_entry, list_t *pos)
{
	__list_add(new_entry, (pos)->prev, (pos));
}

void list_del(list_t *entry)
{
	if (entry == NULL && list_empty(entry))
	{
		return;
	}
	//�������� �޳� entry 
	__list_del(entry->prev, entry->next);
    //���� entry 
	entry->next = (list_t *) NULL;
	entry->prev = (list_t *) NULL;
}

//�������ͷ
bool_t list_empty(list_t *head)
{
	return (((head)->next == head) || (head->next == NULL));
}

//pos -> new_entry -> ((pos)->next)	
//pos -> (pos->next) -> ((pos->next)->next)
list_t *list_front_get(list_t *pos)
{
	if (pos == NULL || list_empty(pos))
	{
		return NULL;
	}

	list_t *temp = (pos)->next;
	//�޳� (pos->next)
	__list_del(pos, temp->next);
    //���� temp Ҳ���� ���� (pos)->next
	temp->next = (list_t *) NULL;
	temp->prev = (list_t *) NULL;

	return temp;
}

//((pos)->prev) -> new_entry -> (pos)
//((pos->prev)->prev) -> (pos->prev) -> (pos)
list_t *list_behind_get(list_t *pos)
{
	if (pos == NULL || list_empty(pos))
	{
		return NULL;
	}
	
	list_t *temp = (pos)->prev;
	//�޳� (pos)->prev
	__list_del(temp->prev, pos);
    //���� temp Ҳ���ǹ��� ((pos)->prev)
	temp->next = (list_t *) NULL;
	temp->prev = (list_t *) NULL;
    
	return temp;
}

//��ѯԲ�����еĳ�Ա����
uint16_t list_count(list_t *head)
{
    uint16_t count=0;
    
	for (list_t *pos = (head)->next; pos != (head); pos = pos->next)
    {
        count++;
    }
    
    return count;
}
	




