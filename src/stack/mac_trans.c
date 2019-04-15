#include <platform.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>

void mac_trans_start(void)
{
	phy_rx_state(PHY_ID_0, PLAT_FALSE);
}

void mac_trans_stop(void)
{
	phy_sleep_state(PHY_ID_0);
}



