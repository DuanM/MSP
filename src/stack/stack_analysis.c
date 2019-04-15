#include <platform.h>
#include <device.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>

bool_t stack_address_analysis(kbuf_t *kbuf)
{
    //OSEL_DECL_CRITICAL();
	if(kbuf == PLAT_NULL) return PLAT_FALSE;
	mac_frm_head_t *mac_frm_head = (mac_frm_head_t *)(kbuf->base+sizeof(phy_frm_head_t));	
//	if(	mac_frm_head->mid_id != mac_pib.mid_id || mac_frm_head->sml_id != mac_pib.sml_id)
//	{ 
//		return PLAT_FALSE;
//	}
	if(	mac_frm_head->src_id == mac_pib.id)
	{ 
		return PLAT_FALSE;
	}
	
	device_info_t *p_device_info = device_info_get();

    if(mac_frm_head->dst_id != mac_pib.id && mac_frm_head->dst_id != 0xff) 
    { 
        return PLAT_FALSE;
    }
	
	return PLAT_TRUE;
}








