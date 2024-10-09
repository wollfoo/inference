/*
 * Copyright © 2010-2022 Inria.  All rights reserved.
 * Copyright © 2011-2012 Université Bordeaux
 * Copyright © 2011 Cisco Systems, Inc.  All rights reserved.
 * See COPYING in top-level directory.
 */

#include "private/autogen/config.h"
#include "hwloc.h"
#include "private/private.h"
#include "private/debug.h"
#include "private/misc.h"

#include <float.h>
#include <math.h>
#include <hwloc.h> // Thêm bọc hàm hwloc vào file gốc

// Hàm kiểm tra hỗ trợ hwloc topology và xử lý khoảng cách dựa vào cấu trúc topology
static int
hwloc_check_distances_support(hwloc_topology_t topology) {
    // Kiểm tra xem hệ thống có hỗ trợ khoảng cách không
    const struct hwloc_topology_support *support = hwloc_topology_get_support(topology);
    if (!support->distances->get) {
        fprintf(stderr, "This topology does not support distances.\n");
        return -1;
    }
    return 0;
}

static struct hwloc_internal_distances_s *
hwloc__internal_distances_from_public(hwloc_topology_t topology, struct hwloc_distances_s *distances);

static void
hwloc__groups_by_distances(struct hwloc_topology *topology, unsigned nbobjs, struct hwloc_obj **objs, uint64_t *values, unsigned long kind, unsigned nbaccuracies, float *accuracies, int needcheck);

static void
hwloc_internal_distances_restrict(hwloc_obj_t *objs,
				  uint64_t *indexes,
				  hwloc_obj_type_t *different_types,
				  uint64_t *values,
				  unsigned nbobjs, unsigned disappeared);

static void
hwloc_internal_distances_print_matrix(struct hwloc_internal_distances_s *dist)
{
  unsigned nbobjs = dist->nbobjs;
  hwloc_obj_t *objs = dist->objs;
  hwloc_uint64_t *values = dist->values;
  int gp = !HWLOC_DIST_TYPE_USE_OS_INDEX(dist->unique_type);
  unsigned i, j;

  fprintf(stderr, "%s", gp ? "gp_index" : "os_index");
  for(j=0; j<nbobjs; j++)
    fprintf(stderr, " % 5d", (int)(gp ? objs[j]->gp_index : objs[j]->os_index));
  fprintf(stderr, "\n");
  for(i=0; i<nbobjs; i++) {
    fprintf(stderr, "  % 5d", (int)(gp ? objs[i]->gp_index : objs[i]->os_index));
    for(j=0; j<nbobjs; j++)
      fprintf(stderr, " % 5lld", (long long) values[i*nbobjs + j]);
    fprintf(stderr, "\n");
  }
}

// Đoạn mã này xử lý việc in khoảng cách và thêm khả năng kiểm tra hỗ trợ hwloc

int hwloc_print_distances(hwloc_topology_t topology) {
    unsigned depth, nbobjs;
    hwloc_obj_type_t type;
    struct hwloc_distances_s *distances;
    unsigned nr = 1;
    int err;

    // Kiểm tra hỗ trợ
    if (hwloc_check_distances_support(topology) != 0) {
        return -1;
    }

    // Duyệt qua tất cả các cấp độ trong topology
    for (depth = 0; ; depth++) {
        type = hwloc_get_depth_type(topology, depth);
        if (type == HWLOC_OBJ_TYPE_NONE) {
            break;
        }

        nbobjs = hwloc_get_nbobjs_by_depth(topology, depth);
        if (!nbobjs)
            continue;

        err = hwloc_distances_get_by_depth(topology, depth, &nr, &distances, 0, 0);
        if (err < 0 || !nr)
            continue;

        hwloc_internal_distances_print_matrix((struct hwloc_internal_distances_s *) distances);
        hwloc_distances_release(topology, distances);
    }

    return 0;
}

// Các hàm toàn cục về khởi tạo, chuẩn bị, hủy và nhân bản
void hwloc_internal_distances_init(struct hwloc_topology *topology)
{
  topology->first_dist = topology->last_dist = NULL;
  topology->next_dist_id = 0;
}

void hwloc_internal_distances_prepare(struct hwloc_topology *topology)
{
  char *env;
  hwloc_localeswitch_declare;

  topology->grouping = 1;
  if (topology->type_filter[HWLOC_OBJ_GROUP] == HWLOC_TYPE_FILTER_KEEP_NONE)
    topology->grouping = 0;
  env = getenv("HWLOC_GROUPING");
  if (env && !atoi(env))
    topology->grouping = 0;

  if (topology->grouping) {
    topology->grouping_next_subkind = 0;

    HWLOC_BUILD_ASSERT(sizeof(topology->grouping_accuracies)/sizeof(*topology->grouping_accuracies) == 5);
    topology->grouping_accuracies[0] = 0.0f;
    topology->grouping_accuracies[1] = 0.01f;
    topology->grouping_accuracies[2] = 0.02f;
    topology->grouping_accuracies[3] = 0.05f;
    topology->grouping_accuracies[4] = 0.1f;
    topology->grouping_nbaccuracies = 5;

    hwloc_localeswitch_init();
    env = getenv("HWLOC_GROUPING_ACCURACY");
    if (!env) {
      topology->grouping_nbaccuracies = 1;
    } else if (strcmp(env, "try")) {
      topology->grouping_nbaccuracies = 1;
      topology->grouping_accuracies[0] = (float) atof(env);
    }
    hwloc_localeswitch_fini();

    topology->grouping_verbose = 0;
    env = getenv("HWLOC_GROUPING_VERBOSE");
    if (env)
      topology->grouping_verbose = atoi(env);
  }
}

static void hwloc_internal_distances_free(struct hwloc_internal_distances_s *dist)
{
  free(dist->name);
  free(dist->different_types);
  free(dist->indexes);
  free(dist->objs);
  free(dist->values);
  free(dist);
}

void hwloc_internal_distances_destroy(struct hwloc_topology * topology)
{
  struct hwloc_internal_distances_s *dist, *next = topology->first_dist;
  while ((dist = next) != NULL) {
    next = dist->next;
    hwloc_internal_distances_free(dist);
  }
  topology->first_dist = topology->last_dist = NULL;
}

int hwloc_internal_distances_dup(struct hwloc_topology *new, struct hwloc_topology *old)
{
  struct hwloc_internal_distances_s *olddist;
  int err;
  new->next_dist_id = old->next_dist_id;
  for(olddist = old->first_dist; olddist; olddist = olddist->next) {
    err = hwloc_internal_distances_dup_one(new, olddist);
    if (err < 0)
      return err;
  }
  return 0;
}

// Nhân bản một khoảng cách từ topologies cũ sang mới
static int hwloc_internal_distances_dup_one(struct hwloc_topology *new, struct hwloc_internal_distances_s *olddist)
{
  struct hwloc_tma *tma = new->tma;
  struct hwloc_internal_distances_s *newdist;
  unsigned nbobjs = olddist->nbobjs;

  newdist = hwloc_tma_malloc(tma, sizeof(*newdist));
  if (!newdist)
    return -1;
  if (olddist->name) {
    newdist->name = hwloc_tma_strdup(tma, olddist->name);
    if (!newdist->name) {
      hwloc_internal_distances_free(newdist);
      return -1;
    }
  } else {
    newdist->name = NULL;
  }

  if (olddist->different_types) {
    newdist->different_types = hwloc_tma_malloc(tma, nbobjs * sizeof(*newdist->different_types));
    if (!newdist->different_types) {
      hwloc_internal_distances_free(newdist);
      return -1;
    }
    memcpy(newdist->different_types, olddist->different_types, nbobjs * sizeof(*newdist->different_types));
  } else
    newdist->different_types = NULL;

  newdist->unique_type = olddist->unique_type;
  newdist->nbobjs = nbobjs;
  newdist->kind = olddist->kind;
  newdist->id = olddist->id;
  
  return 0;
}

