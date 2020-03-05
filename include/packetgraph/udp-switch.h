/* Copyright 2020 Outscale SAS
 *
 * This file is part of Packetgraph.
 *
 * Packetgraph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Packetgraph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Packetgraph.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PG_UDP_SWITCH_H_
#define PG_UDP_SWITCH_H_

#include <packetgraph/errors.h>

#define PG_UDP_SWITCH_DEFAULT_PG_PORT 0

#define PG_USP_FILTER_DST_PORT 1 << 0
#define PG_USP_FILTER_SRC_PORT 1 << 1

struct pg_udp_port_sw_info {
	uint16_t udp_src_port;
	uint16_t udp_dst_port;
	uint32_t flag;
};

/**
 * Create a new hub brick
 *
 * @param	name name of the brick
 * @param	west_max maximum of links you can connect on the west side
 * @param	east_max maximum of links you can connect on the east side
 * @param	ports array of pg_udp_port_sw_info that describe where
 *		packets goes
 * @param	errp is set in case of an error
 * @return	a pointer to a brick structure on success, NULL on error
 */
PG_WARN_UNUSED
struct pg_brick *pg_udp_port_switch_new(const char *name,
					uint32_t west_max,
					uint32_t east_max,
					struct pg_udp_port_sw_info ports[],
					int nb_filters,
					enum pg_side output,
					struct pg_error **errp);


#endif  /* PG_UDP_SWITCH_H_ */

