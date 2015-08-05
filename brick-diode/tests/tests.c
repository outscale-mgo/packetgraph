/* Copyright 2014 Nodalink EURL
 *
 * This file is part of Butterfly.
 *
 * Butterfly is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Butterfly is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Butterfly.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <string.h>
#include <packetgraph/packetgraph.h>
#include <packetgraph/diode.h>
#include <packetgraph/brick.h>
#include <packetgraph/packets.h>
#include <packetgraph/utils/mempool.h>
#include <packetgraph/utils/bitmask.h>
#include <packetgraph/collect.h>

#define NB_PKTS 3

#define	TEST_DIODE_INIT(output)						\
	struct brick *node1, *node2, *collect_west, *collect_east;	\
	struct rte_mbuf *pkts[MAX_PKTS_BURST], **result_pkts;		\
	struct rte_mempool *mp = get_mempool();				\
	struct switch_error *error = NULL;				\
	uint16_t i;							\
	uint64_t pkts_mask;						\
	memset(pkts, 0, MAX_PKTS_BURST * sizeof(struct rte_mbuf *));	\
	for (i = 0; i < NB_PKTS; i++) {					\
		pkts[i] = rte_pktmbuf_alloc(mp);			\
		g_assert(pkts[i]);					\
		pkts[i]->udata64 = i;					\
	}								\
	node1 = diode_new("node1", 4, 4, output, &error);		\
	g_assert(!error);						\
	node2 = diode_new("node2", 4, 4, output, &error);		\
	g_assert(!error);						\
	collect_west = collect_new("cwest", 4, 4, &error);		\
	g_assert(!error);						\
	g_assert(collect_west);						\
	collect_east = collect_new("ceast", 4, 4, &error);		\
	g_assert(!error);						\
	g_assert(collect_east);						\
	brick_link(collect_west, node1, &error);				\
	g_assert(!error);						\
	brick_link(node1, node2, &error);				\
	g_assert(!error);						\
	brick_link(node2, collect_east, &error);			\
	g_assert(!error)

#define	TEST_DIODE_DESTROY() do {				\
		brick_unlink(node1, &error);			\
		g_assert(!error);				\
		brick_unlink(node2, &error);			\
		g_assert(!error);				\
		brick_unlink(collect_west, &error);		\
		g_assert(!error);				\
		brick_unlink(collect_east, &error);		\
		g_assert(!error);				\
		packets_free(pkts, mask_firsts(NB_PKTS));	\
		brick_decref(node1, &error);			\
		g_assert(!error);				\
		brick_decref(node2, &error);			\
		g_assert(!error);				\
		brick_reset(collect_west, &error);		\
		g_assert(!error);				\
		brick_reset(collect_east, &error);		\
		g_assert(!error);				\
		brick_decref(collect_west, &error);		\
		g_assert(!error);				\
		brick_decref(collect_east, &error);		\
		g_assert(!error);				\
	} while (0)

#define	DIODE_TEST(get_burst_fn, to_collect, result_to_check) do {	    \
		result_pkts = get_burst_fn(to_collect, &pkts_mask, &error); \
		g_assert(!error);					    \
		g_assert(pkts_mask == mask_firsts(result_to_check));	    \
		if (result_to_check)					    \
			for (i = 0; i < NB_PKTS; i++)			    \
				g_assert(result_pkts[i]->udata64 == i);	    \
		else							    \
			g_assert(!result_pkts);				    \
	} while (0)

static void test_diode_west_good_direction(void)
{
	TEST_DIODE_INIT(WEST_SIDE);

	/* send a burst to the west from the eastest nope brick */
	brick_burst_to_west(node2, 0, pkts, NB_PKTS, mask_firsts(NB_PKTS),
			    &error);

	/* check no packet ended on the east */
	DIODE_TEST(brick_west_burst_get, collect_east, 0);
	DIODE_TEST(brick_east_burst_get, collect_east, 0);

	/* collect pkts on the west */
	DIODE_TEST(brick_west_burst_get, collect_west, 0);
	DIODE_TEST(brick_east_burst_get, collect_west, 3);

	TEST_DIODE_DESTROY();
}

static void test_diode_west_bad_direction(void)
{
	TEST_DIODE_INIT(EAST_SIDE);

	/* send a burst to the west from the eastest nope brick */
	brick_burst_to_west(node2, 0, pkts, NB_PKTS, mask_firsts(NB_PKTS),
			    &error);

	/* check no packet ended */
	DIODE_TEST(brick_west_burst_get, collect_east, 0);
	DIODE_TEST(brick_east_burst_get, collect_east, 0);
	DIODE_TEST(brick_west_burst_get, collect_west, 0);
	DIODE_TEST(brick_east_burst_get, collect_west, 0);

	TEST_DIODE_DESTROY();
}

static void test_diode_east_good_direction(void)
{
	TEST_DIODE_INIT(EAST_SIDE);

	/* send a burst to the east from the westest nope brick */
	brick_burst_to_east(node1, 0, pkts, NB_PKTS, mask_firsts(NB_PKTS),
			    &error);

	/* check no packet ended on the west */
	DIODE_TEST(brick_west_burst_get, collect_west, 0);
	DIODE_TEST(brick_east_burst_get, collect_west, 0);

	/* collect pkts on the east */
	DIODE_TEST(brick_east_burst_get, collect_east, 0);
	DIODE_TEST(brick_west_burst_get, collect_east, 3);

	TEST_DIODE_DESTROY();
}

static void test_diode_east_bad_direction(void)
{
	TEST_DIODE_INIT(WEST_SIDE);

	/* send a burst to the east from the westest nope brick */
	brick_burst_to_east(node1, 0, pkts, NB_PKTS, mask_firsts(NB_PKTS),
			    &error);

	/* check no packet ended */
	DIODE_TEST(brick_west_burst_get, collect_east, 0);
	DIODE_TEST(brick_east_burst_get, collect_east, 0);
	DIODE_TEST(brick_west_burst_get, collect_west, 0);
	DIODE_TEST(brick_east_burst_get, collect_west, 0);

	TEST_DIODE_DESTROY();
}

#undef	DIODE_TEST
#undef	TEST_DIODE_INIT
#undef	TEST_DIODE_DESTROY

static void test_diode(void)
{
	/* tests in the same order as the header function declarations */
	g_test_add_func("/brick/diode/west/good",
			test_diode_west_good_direction);
	g_test_add_func("/brick/diode/west/bad",
			test_diode_west_bad_direction);
	g_test_add_func("/brick/diode/east/good",
			test_diode_east_good_direction);
	g_test_add_func("/brick/diode/east/bad",
			test_diode_east_bad_direction);
}

int main(int argc, char **argv)
{
	struct switch_error *error;
	int r;

	/* tests in the same order as the header function declarations */
	g_test_init(&argc, &argv, NULL);

	/* initialize packetgraph */
	packetgraph_start(argc, argv, &error);
	g_assert(!error);

	test_diode();
	r = g_test_run();

	packetgraph_stop();
	return r;
}