#pragma once
#include "ndcExtDev.h"

void init_net_if();
void term_net_if();

u32 net_if_rx_pending();
void net_if_rx(u8* data,u32 count);
void net_if_tx(u8* data,u32 count);
