

#ifndef IEEE80211S_H
#define IEEE80211S_H

#include <linux/types.h>
#include <linux/jhash.h>
#include <asm/unaligned.h>
#include "ieee80211_i.h"


/* Data structures */

enum mesh_path_flags {
	MESH_PATH_ACTIVE =	BIT(0),
	MESH_PATH_RESOLVING =	BIT(1),
	MESH_PATH_SN_VALID =	BIT(2),
	MESH_PATH_FIXED	=	BIT(3),
	MESH_PATH_RESOLVED =	BIT(4),
};

enum mesh_deferred_task_flags {
	MESH_WORK_HOUSEKEEPING,
	MESH_WORK_GROW_MPATH_TABLE,
	MESH_WORK_GROW_MPP_TABLE,
	MESH_WORK_ROOT,
};

struct mesh_path {
	u8 dst[ETH_ALEN];
	u8 mpp[ETH_ALEN];	/* used for MPP or MAP */
	struct ieee80211_sub_if_data *sdata;
	struct sta_info *next_hop;
	struct timer_list timer;
	struct sk_buff_head frame_queue;
	struct rcu_head rcu;
	u32 sn;
	u32 metric;
	u8 hop_count;
	unsigned long exp_time;
	u32 discovery_timeout;
	u8 discovery_retries;
	enum mesh_path_flags flags;
	spinlock_t state_lock;
};

struct mesh_table {
	/* Number of buckets will be 2^N */
	struct hlist_head *hash_buckets;
	spinlock_t *hashwlock;		/* One per bucket, for add/del */
	unsigned int hash_mask;		/* (2^size_order) - 1 */
	__u32 hash_rnd;			/* Used for hash generation */
	atomic_t entries;		/* Up to MAX_MESH_NEIGHBOURS */
	void (*free_node) (struct hlist_node *p, bool free_leafs);
	int (*copy_node) (struct hlist_node *p, struct mesh_table *newtbl);
	int size_order;
	int mean_chain_len;
};

/* Recent multicast cache */
/* RMC_BUCKETS must be a power of 2, maximum 256 */
#define RMC_BUCKETS		256
#define RMC_QUEUE_MAX_LEN	4
#define RMC_TIMEOUT		(3 * HZ)

struct rmc_entry {
	struct list_head list;
	u32 seqnum;
	unsigned long exp_time;
	u8 sa[ETH_ALEN];
};

struct mesh_rmc {
	struct rmc_entry bucket[RMC_BUCKETS];
	u32 idx_mask;
};


#define MESH_CFG_CMP_LEN 	(IEEE80211_MESH_CONFIG_LEN - 2)

/* Default values, timeouts in ms */
#define MESH_TTL 		31
#define MESH_MAX_RETR	 	3
#define MESH_RET_T 		100
#define MESH_CONF_T 		100
#define MESH_HOLD_T 		100

#define MESH_PATH_TIMEOUT	5000
#define MESH_PREQ_MIN_INT	10
#define MESH_DIAM_TRAVERSAL_TIME 50
#define MESH_PATH_REFRESH_TIME			1000
#define MESH_MIN_DISCOVERY_TIMEOUT (2 * MESH_DIAM_TRAVERSAL_TIME)
#define MESH_DEFAULT_BEACON_INTERVAL		1000 	/* in 1024 us units */

#define MESH_MAX_PREQ_RETRIES 4
#define MESH_PATH_EXPIRE (600 * HZ)

/* Default maximum number of established plinks per interface */
#define MESH_MAX_ESTAB_PLINKS	32

/* Default maximum number of plinks per interface */
#define MESH_MAX_PLINKS		256

/* Maximum number of paths per interface */
#define MESH_MAX_MPATHS		1024

/* Pending ANA approval */
#define MESH_PATH_SEL_ACTION	0

/* PERR reason codes */
#define PEER_RCODE_UNSPECIFIED  11
#define PERR_RCODE_NO_ROUTE     12
#define PERR_RCODE_DEST_UNREACH 13

/* Public interfaces */
/* Various */
int ieee80211_fill_mesh_addresses(struct ieee80211_hdr *hdr, __le16 *fc,
				  const u8 *da, const u8 *sa);
int ieee80211_new_mesh_header(struct ieee80211s_hdr *meshhdr,
		struct ieee80211_sub_if_data *sdata, char *addr4,
		char *addr5, char *addr6);
int mesh_rmc_check(u8 *addr, struct ieee80211s_hdr *mesh_hdr,
		struct ieee80211_sub_if_data *sdata);
bool mesh_matches_local(struct ieee802_11_elems *ie,
		struct ieee80211_sub_if_data *sdata);
void mesh_ids_set_default(struct ieee80211_if_mesh *mesh);
void mesh_mgmt_ies_add(struct sk_buff *skb,
		struct ieee80211_sub_if_data *sdata);
void mesh_rmc_free(struct ieee80211_sub_if_data *sdata);
int mesh_rmc_init(struct ieee80211_sub_if_data *sdata);
void ieee80211s_init(void);
void ieee80211s_update_metric(struct ieee80211_local *local,
		struct sta_info *stainfo, struct sk_buff *skb);
void ieee80211s_stop(void);
void ieee80211_mesh_init_sdata(struct ieee80211_sub_if_data *sdata);
ieee80211_rx_result
ieee80211_mesh_rx_mgmt(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb);
void ieee80211_start_mesh(struct ieee80211_sub_if_data *sdata);
void ieee80211_stop_mesh(struct ieee80211_sub_if_data *sdata);
void ieee80211_mesh_root_setup(struct ieee80211_if_mesh *ifmsh);

/* Mesh paths */
int mesh_nexthop_lookup(struct sk_buff *skb,
		struct ieee80211_sub_if_data *sdata);
void mesh_path_start_discovery(struct ieee80211_sub_if_data *sdata);
struct mesh_path *mesh_path_lookup(u8 *dst,
		struct ieee80211_sub_if_data *sdata);
struct mesh_path *mpp_path_lookup(u8 *dst,
				  struct ieee80211_sub_if_data *sdata);
int mpp_path_add(u8 *dst, u8 *mpp, struct ieee80211_sub_if_data *sdata);
struct mesh_path *mesh_path_lookup_by_idx(int idx,
		struct ieee80211_sub_if_data *sdata);
void mesh_path_fix_nexthop(struct mesh_path *mpath, struct sta_info *next_hop);
void mesh_path_expire(struct ieee80211_sub_if_data *sdata);
void mesh_path_flush(struct ieee80211_sub_if_data *sdata);
void mesh_rx_path_sel_frame(struct ieee80211_sub_if_data *sdata,
		struct ieee80211_mgmt *mgmt, size_t len);
int mesh_path_add(u8 *dst, struct ieee80211_sub_if_data *sdata);
/* Mesh plinks */
void mesh_neighbour_update(u8 *hw_addr, u32 rates,
		struct ieee80211_sub_if_data *sdata, bool add);
bool mesh_peer_accepts_plinks(struct ieee802_11_elems *ie);
void mesh_accept_plinks_update(struct ieee80211_sub_if_data *sdata);
void mesh_plink_broken(struct sta_info *sta);
void mesh_plink_deactivate(struct sta_info *sta);
int mesh_plink_open(struct sta_info *sta);
void mesh_plink_block(struct sta_info *sta);
void mesh_rx_plink_frame(struct ieee80211_sub_if_data *sdata,
			 struct ieee80211_mgmt *mgmt, size_t len,
			 struct ieee80211_rx_status *rx_status);

/* Private interfaces */
/* Mesh tables */
struct mesh_table *mesh_table_alloc(int size_order);
void mesh_table_free(struct mesh_table *tbl, bool free_leafs);
void mesh_mpath_table_grow(void);
void mesh_mpp_table_grow(void);
u32 mesh_table_hash(u8 *addr, struct ieee80211_sub_if_data *sdata,
		struct mesh_table *tbl);
/* Mesh paths */
int mesh_path_error_tx(u8 ttl, u8 *target, __le32 target_sn, __le16 target_rcode,
		       const u8 *ra, struct ieee80211_sub_if_data *sdata);
void mesh_path_assign_nexthop(struct mesh_path *mpath, struct sta_info *sta);
void mesh_path_flush_pending(struct mesh_path *mpath);
void mesh_path_tx_pending(struct mesh_path *mpath);
int mesh_pathtbl_init(void);
void mesh_pathtbl_unregister(void);
int mesh_path_del(u8 *addr, struct ieee80211_sub_if_data *sdata);
void mesh_path_timer(unsigned long data);
void mesh_path_flush_by_nexthop(struct sta_info *sta);
void mesh_path_discard_frame(struct sk_buff *skb,
		struct ieee80211_sub_if_data *sdata);
void mesh_path_quiesce(struct ieee80211_sub_if_data *sdata);
void mesh_path_restart(struct ieee80211_sub_if_data *sdata);
void mesh_path_tx_root_frame(struct ieee80211_sub_if_data *sdata);

extern int mesh_paths_generation;

#ifdef CONFIG_MAC80211_MESH
extern int mesh_allocated;

static inline int mesh_plink_free_count(struct ieee80211_sub_if_data *sdata)
{
	return sdata->u.mesh.mshcfg.dot11MeshMaxPeerLinks -
	       atomic_read(&sdata->u.mesh.mshstats.estab_plinks);
}

static inline bool mesh_plink_availables(struct ieee80211_sub_if_data *sdata)
{
	return (min_t(long, mesh_plink_free_count(sdata),
		   MESH_MAX_PLINKS - sdata->local->num_sta)) > 0;
}

static inline void mesh_path_activate(struct mesh_path *mpath)
{
	mpath->flags |= MESH_PATH_ACTIVE | MESH_PATH_RESOLVED;
}

#define for_each_mesh_entry(x, p, node, i) \
	for (i = 0; i <= x->hash_mask; i++) \
		hlist_for_each_entry_rcu(node, p, &x->hash_buckets[i], list)

void ieee80211_mesh_notify_scan_completed(struct ieee80211_local *local);

void ieee80211_mesh_quiesce(struct ieee80211_sub_if_data *sdata);
void ieee80211_mesh_restart(struct ieee80211_sub_if_data *sdata);
void mesh_plink_quiesce(struct sta_info *sta);
void mesh_plink_restart(struct sta_info *sta);
#else
#define mesh_allocated	0
static inline void
ieee80211_mesh_notify_scan_completed(struct ieee80211_local *local) {}
static inline void ieee80211_mesh_quiesce(struct ieee80211_sub_if_data *sdata)
{}
static inline void ieee80211_mesh_restart(struct ieee80211_sub_if_data *sdata)
{}
static inline void mesh_plink_quiesce(struct sta_info *sta) {}
static inline void mesh_plink_restart(struct sta_info *sta) {}
#endif

#endif /* IEEE80211S_H */