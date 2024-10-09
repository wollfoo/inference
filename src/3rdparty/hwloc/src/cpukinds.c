/*
 * Copyright © 2020-2022 Inria.  All rights reserved.
 * See COPYING in top-level directory.
 */

#include "private/autogen/config.h"
#include "hwloc.h"
#include "private/private.h"
#include "private/debug.h"
#include <hwloc.h> // Thêm bọc hàm hwloc vào file gốc

/*****************
 * Basics
 */

// Khởi tạo cpukinds trong topology
void
hwloc_internal_cpukinds_init(struct hwloc_topology *topology)
{
  topology->cpukinds = NULL;
  topology->nr_cpukinds = 0;
  topology->nr_cpukinds_allocated = 0;
}

// Giải phóng tài nguyên cpukinds trong topology
void
hwloc_internal_cpukinds_destroy(struct hwloc_topology *topology)
{
  unsigned i;
  for(i=0; i<topology->nr_cpukinds; i++) {
    struct hwloc_internal_cpukind_s *kind = &topology->cpukinds[i];
    hwloc_bitmap_free(kind->cpuset);
    hwloc__free_infos(kind->infos, kind->nr_infos);
  }
  free(topology->cpukinds);
  topology->cpukinds = NULL;
  topology->nr_cpukinds = 0;
}

// Nhân bản thông tin cpukinds từ topologies cũ sang mới
int
hwloc_internal_cpukinds_dup(hwloc_topology_t new, hwloc_topology_t old)
{
  struct hwloc_tma *tma = new->tma;
  struct hwloc_internal_cpukind_s *kinds;
  unsigned i;

  if (!old->nr_cpukinds)
    return 0;

  kinds = hwloc_tma_malloc(tma, old->nr_cpukinds * sizeof(*kinds));
  if (!kinds)
    return -1;
  new->cpukinds = kinds;
  new->nr_cpukinds = old->nr_cpukinds;
  memcpy(kinds, old->cpukinds, old->nr_cpukinds * sizeof(*kinds));

  for(i=0;i<old->nr_cpukinds; i++) {
    kinds[i].cpuset = hwloc_bitmap_tma_dup(tma, old->cpukinds[i].cpuset);
    if (!kinds[i].cpuset) {
      new->nr_cpukinds = i;
      goto failed;
    }
    if (hwloc__tma_dup_infos(tma,
                             &kinds[i].infos, &kinds[i].nr_infos,
                             old->cpukinds[i].infos, old->cpukinds[i].nr_infos) < 0) {
      assert(!tma || !tma->dontfree); /* this tma cannot fail to allocate */
      hwloc_bitmap_free(kinds[i].cpuset);
      new->nr_cpukinds = i;
      goto failed;
    }
  }

  return 0;

 failed:
  hwloc_internal_cpukinds_destroy(new);
  return -1;
}

// Giới hạn lại thông tin cpukinds trong topology
void
hwloc_internal_cpukinds_restrict(hwloc_topology_t topology)
{
  unsigned i;
  int removed = 0;
  for(i=0; i<topology->nr_cpukinds; i++) {
    struct hwloc_internal_cpukind_s *kind = &topology->cpukinds[i];
    hwloc_bitmap_and(kind->cpuset, kind->cpuset, hwloc_get_root_obj(topology)->cpuset);
    if (hwloc_bitmap_iszero(kind->cpuset)) {
      hwloc_bitmap_free(kind->cpuset);
      hwloc__free_infos(kind->infos, kind->nr_infos);
      memmove(kind, kind+1, (topology->nr_cpukinds - i - 1)*sizeof(*kind));
      i--;
      topology->nr_cpukinds--;
      removed = 1;
    }
  }
  if (removed)
    hwloc_internal_cpukinds_rank(topology);
}

/********************
 * Registering
 */

// Kiểm tra sự trùng lặp thông tin cpukind
static __hwloc_inline int
hwloc__cpukind_check_duplicate_info(struct hwloc_internal_cpukind_s *kind,
                                    const char *name, const char *value)
{
  unsigned i;
  for(i=0; i<kind->nr_infos; i++)
    if (!strcmp(kind->infos[i].name, name)
        && !strcmp(kind->infos[i].value, value))
      return 1;
  return 0;
}

// Thêm thông tin mới cho cpukind
static __hwloc_inline void
hwloc__cpukind_add_infos(struct hwloc_internal_cpukind_s *kind,
                         const struct hwloc_info_s *infos, unsigned nr_infos)
{
  unsigned i;
  for(i=0; i<nr_infos; i++) {
    if (hwloc__cpukind_check_duplicate_info(kind, infos[i].name, infos[i].value))
      continue;
    hwloc__add_info(&kind->infos, &kind->nr_infos, infos[i].name, infos[i].value);
  }
}

// Đăng ký cpukinds mới trong topology
int
hwloc_internal_cpukinds_register(hwloc_topology_t topology, hwloc_cpuset_t cpuset,
                                 int forced_efficiency,
                                 const struct hwloc_info_s *infos, unsigned nr_infos,
                                 unsigned long flags)
{
  struct hwloc_internal_cpukind_s *kinds;
  unsigned i, max, bits, oldnr, newnr;

  if (hwloc_bitmap_iszero(cpuset)) {
    hwloc_bitmap_free(cpuset);
    errno = EINVAL;
    return -1;
  }

  if (flags & ~HWLOC_CPUKINDS_REGISTER_FLAG_OVERWRITE_FORCED_EFFICIENCY) {
    errno = EINVAL;
    return -1;
  }

  /* Nếu hiện có N loại, chúng ta có thể cần 2N+1 loại mới sau khi chèn loại mới */
  max = 2 * topology->nr_cpukinds + 1;
  bits = hwloc_flsl(max-1) + 1;
  max = 1U<<bits;
  if (max < 8)
    max = 8;

  kinds = topology->cpukinds;
  if (max > topology->nr_cpukinds_allocated) {
    kinds = realloc(kinds, max * sizeof(*kinds));
    if (!kinds) {
      hwloc_bitmap_free(cpuset);
      return -1;
    }
    memset(&kinds[topology->nr_cpukinds_allocated], 0, (max - topology->nr_cpukinds_allocated) * sizeof(*kinds));
    topology->nr_cpukinds_allocated = max;
    topology->cpukinds = kinds;
  }

  newnr = oldnr = topology->nr_cpukinds;
  for(i=0; i<oldnr; i++) {
    int res = hwloc_bitmap_compare_inclusion(cpuset, kinds[i].cpuset);
    if (res == HWLOC_BITMAP_INTERSECTS || res == HWLOC_BITMAP_INCLUDED) {
      kinds[newnr].cpuset = hwloc_bitmap_alloc();
      kinds[newnr].efficiency = HWLOC_CPUKIND_EFFICIENCY_UNKNOWN;
      kinds[newnr].forced_efficiency = forced_efficiency;
      hwloc_bitmap_and(kinds[newnr].cpuset, cpuset, kinds[i].cpuset);
      hwloc__cpukind_add_infos(&kinds[newnr], kinds[i].infos, kinds[i].nr_infos);
      hwloc__cpukind_add_infos(&kinds[newnr], infos, nr_infos);
      hwloc_bitmap_andnot(kinds[i].cpuset, kinds[i].cpuset, kinds[newnr].cpuset);
      hwloc_bitmap_andnot(cpuset, cpuset, kinds[newnr].cpuset);

      newnr++;

    } else if (res == HWLOC_BITMAP_CONTAINS
               || res == HWLOC_BITMAP_EQUAL) {
      hwloc__cpukind_add_infos(&kinds[i], infos, nr_infos);
      if ((flags & HWLOC_CPUKINDS_REGISTER_FLAG_OVERWRITE_FORCED_EFFICIENCY)
          || kinds[i].forced_efficiency == HWLOC_CPUKIND_EFFICIENCY_UNKNOWN)
        kinds[i].forced_efficiency = forced_efficiency;
      hwloc_bitmap_andnot(cpuset, cpuset, kinds[i].cpuset);

    } else {
      assert(res == HWLOC_BITMAP_DIFFERENT);
    }

    if (hwloc_bitmap_iszero(cpuset))
      break;
  }

  if (!hwloc_bitmap_iszero(cpuset)) {
    kinds[newnr].cpuset = cpuset;
    kinds[newnr].efficiency = HWLOC_CPUKIND_EFFICIENCY_UNKNOWN;
    kinds[newnr].forced_efficiency = forced_efficiency;
    hwloc__cpukind_add_infos(&kinds[newnr], infos, nr_infos);
    newnr++;
  } else {
    hwloc_bitmap_free(cpuset);
  }

  topology->nr_cpukinds = newnr;
  return 0;
}

// Đăng ký cpukind mới với việc xử lý và kiểm tra các giá trị hiệu suất đã ép buộc
int
hwloc_cpukinds_register(hwloc_topology_t topology, hwloc_cpuset_t _cpuset,
                        int forced_efficiency,
                        unsigned nr_infos, struct hwloc_info_s *infos,
                        unsigned long flags)
{
  hwloc_bitmap_t cpuset;
  int err;

  if (flags) {
    errno = EINVAL;
    return -1;
  }

  if (!_cpuset || hwloc_bitmap_iszero(_cpuset)) {
    errno = EINVAL;
    return -1;
  }

  cpuset = hwloc_bitmap_dup(_cpuset);
  if (!cpuset)
    return -1;

  if (forced_efficiency < 0)
    forced_efficiency = HWLOC_CPUKIND_EFFICIENCY_UNKNOWN;

  err = hwloc_internal_cpukinds_register(topology, cpuset, forced_efficiency, infos, nr_infos, HWLOC_CPUKINDS_REGISTER_FLAG_OVERWRITE_FORCED_EFFICIENCY);
  if (err < 0)
    return err;

  hwloc_internal_cpukinds_rank(topology);
  return 0;
}

/*********************
 * Ranking
 */

// Xử lý thông tin tóm tắt để phân loại các cpukind
static void
hwloc__cpukinds_summarize_info(struct hwloc_topology *topology,
                               struct hwloc_cpukinds_info_summary *summary)
{
  unsigned i, j;

  summary->have_max_freq = 1;
  summary->have_base_freq = 1;
  summary->have_intel_core_type = 1;

  for(i=0; i<topology->nr_cpukinds; i++) {
    struct hwloc_internal_cpukind_s *kind = &topology->cpukinds[i];
    for(j=0; j<kind->nr_infos; j++) {
      struct hwloc_info_s *info = &kind->infos[j];
      if (!strcmp(info->name, "FrequencyMaxMHz")) {
        summary->summaries[i].max_freq = atoi(info->value);
      } else if (!strcmp(info->name, "FrequencyBaseMHz")) {
        summary->summaries[i].base_freq = atoi(info->value);
      } else if (!strcmp(info->name, "CoreType")) {
        if (!strcmp(info->value, "IntelAtom"))
          summary->summaries[i].intel_core_type = 1;
        else if (!strcmp(info->value, "IntelCore"))
          summary->summaries[i].intel_core_type = 2;
      }
    }
    hwloc_debug("cpukind #%u has intel_core_type %u max_freq %u base_freq %u\n",
                i, summary->summaries[i].intel_core_type,
                summary->summaries[i].max_freq, summary->summaries[i].base_freq);
    if (!summary->summaries[i].base_freq)
      summary->have_base_freq = 0;
    if (!summary->summaries[i].max_freq)
      summary->have_max_freq = 0;
    if (!summary->summaries[i].intel_core_type)
      summary->have_intel_core_type = 0;
  }
}

