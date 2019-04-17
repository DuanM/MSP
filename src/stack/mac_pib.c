#include <platform.h>
#include <stack_priv.h>
#include <mac.h>
#include <device.h>

mac_pib_t mac_pib;

void mac_pib_init(void)
{
	device_info_t *p_device_info = device_info_get();
	mac_pib.id = GET_DEV_ID(p_device_info->id); 
	mac_pib.group_id = GET_DEV_GROUP_ID(p_device_info->id);
	mac_pib.mode_id = GET_DEV_MODE_ID(p_device_info->id);
	mac_pib.type_id = GET_DEV_TYPE_ID(p_device_info->id);
	mac_pib.centre_id = DEV_NET_DISADD;
	mem_set(&nwk_param, 0, sizeof(nwk_param_t));
}


