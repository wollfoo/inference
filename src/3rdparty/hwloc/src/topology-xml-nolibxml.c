/*
 * Copyright © 2009 CNRS
 * Copyright © 2009-2020 Inria.  All rights reserved.
 * Copyright © 2009-2011 Université Bordeaux
 * Copyright © 2009-2011 Cisco Systems, Inc.  All rights reserved.
 * See COPYING in top-level directory.
 */

#include "private/autogen/config.h"
#include "hwloc.h"
#include "hwloc/plugins.h"
#include "private/private.h"
#include "private/misc.h"
#include "private/xml.h"
#include "private/debug.h"

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*******************
 * Import routines *
 *******************/

struct hwloc__nolibxml_backend_data_s {
  size_t buflen; /* size of both buffer, set during backend_init() */
  char *buffer; /* allocated and filled during backend_init() */
};

typedef struct hwloc__nolibxml_import_state_data_s {
  char *tagbuffer; /* buffer containing the next tag */
  char *attrbuffer; /* buffer containing the next attribute of the current node */
  const char *tagname; /* tag name of the current node */
  int closed; /* set if the current node is auto-closing */
} __hwloc_attribute_may_alias * hwloc__nolibxml_import_state_data_t;

/* Wrapper functions for hwloc */

/* Wrapper for hwloc__nolibxml_import_ignore_spaces */
static char *
wrap_hwloc__nolibxml_import_ignore_spaces(char *buffer) {
    return hwloc__nolibxml_import_ignore_spaces(buffer);
}

/* Wrapper for hwloc__nolibxml_import_next_attr */
static int
wrap_hwloc__nolibxml_import_next_attr(hwloc__xml_import_state_t state, char **namep, char **valuep) {
    return hwloc__nolibxml_import_next_attr(state, namep, valuep);
}

/* Wrapper for hwloc__nolibxml_import_find_child */
static int
wrap_hwloc__nolibxml_import_find_child(hwloc__xml_import_state_t state, hwloc__xml_import_state_t childstate, char **tagp) {
    return hwloc__nolibxml_import_find_child(state, childstate, tagp);
}

/* Wrapper for hwloc__nolibxml_import_close_tag */
static int
wrap_hwloc__nolibxml_import_close_tag(hwloc__xml_import_state_t state) {
    return hwloc__nolibxml_import_close_tag(state);
}

/* Wrapper for hwloc__nolibxml_import_close_child */
static void
wrap_hwloc__nolibxml_import_close_child(hwloc__xml_import_state_t state) {
    hwloc__nolibxml_import_close_child(state);
}

/* Wrapper for hwloc__nolibxml_import_get_content */
static int
wrap_hwloc__nolibxml_import_get_content(hwloc__xml_import_state_t state, const char **beginp, size_t expected_length) {
    return hwloc__nolibxml_import_get_content(state, beginp, expected_length);
}

/* Wrapper for hwloc__nolibxml_import_close_content */
static void
wrap_hwloc__nolibxml_import_close_content(hwloc__xml_import_state_t state) {
    hwloc__nolibxml_import_close_content(state);
}

/* Wrapper for hwloc_nolibxml_look_init */
static int
wrap_hwloc_nolibxml_look_init(struct hwloc_xml_backend_data_s *bdata, struct hwloc__xml_import_state_s *state) {
    return hwloc_nolibxml_look_init(bdata, state);
}

/* Wrapper for hwloc_nolibxml_free_buffers */
static void
wrap_hwloc_nolibxml_free_buffers(struct hwloc_xml_backend_data_s *bdata) {
    hwloc_nolibxml_free_buffers(bdata);
}

/* Wrapper for hwloc_nolibxml_look_done */
static void
wrap_hwloc_nolibxml_look_done(struct hwloc_xml_backend_data_s *bdata, int result) {
    hwloc_nolibxml_look_done(bdata, result);
}

/* Wrapper for hwloc_nolibxml_backend_exit */
static void
wrap_hwloc_nolibxml_backend_exit(struct hwloc_xml_backend_data_s *bdata) {
    hwloc_nolibxml_backend_exit(bdata);
}

/* Wrapper for hwloc_nolibxml_read_file */
static int
wrap_hwloc_nolibxml_read_file(const char *xmlpath, char **bufferp, size_t *buflenp) {
    return hwloc_nolibxml_read_file(xmlpath, bufferp, buflenp);
}

/* Wrapper for hwloc_nolibxml_backend_init */
static int
wrap_hwloc_nolibxml_backend_init(struct hwloc_xml_backend_data_s *bdata, const char *xmlpath, const char *xmlbuffer, int xmlbuflen) {
    return hwloc_nolibxml_backend_init(bdata, xmlpath, xmlbuffer, xmlbuflen);
}

/* Wrapper for hwloc_nolibxml_import_diff */
static int
wrap_hwloc_nolibxml_import_diff(struct hwloc__xml_import_state_s *state, const char *xmlpath, const char *xmlbuffer, int xmlbuflen, hwloc_topology_diff_t *firstdiffp, char **refnamep) {
    return hwloc_nolibxml_import_diff(state, xmlpath, xmlbuffer, xmlbuflen, firstdiffp, refnamep);
}

/* Wrapper for hwloc_nolibxml_export_file */
static int
wrap_hwloc_nolibxml_export_file(hwloc_topology_t topology, struct hwloc__xml_export_data_s *edata, const char *filename, unsigned long flags) {
    return hwloc_nolibxml_export_file(topology, edata, filename, flags);
}

/* Wrapper for hwloc_nolibxml_export_buffer */
static int
wrap_hwloc_nolibxml_export_buffer(hwloc_topology_t topology, struct hwloc__xml_export_data_s *edata, char **bufferp, int *buflenp, unsigned long flags) {
    return hwloc_nolibxml_export_buffer(topology, edata, bufferp, buflenp, flags);
}

/* Wrapper for hwloc_nolibxml_free_buffer */
static void
wrap_hwloc_nolibxml_free_buffer(void *xmlbuffer) {
    hwloc_nolibxml_free_buffer(xmlbuffer);
}

/* Wrapper for hwloc_nolibxml_export_diff_file */
static int
wrap_hwloc_nolibxml_export_diff_file(hwloc_topology_diff_t diff, const char *refname, const char *filename) {
    return hwloc_nolibxml_export_diff_file(diff, refname, filename);
}

/* Wrapper for hwloc_nolibxml_export_diff_buffer */
static int
wrap_hwloc_nolibxml_export_diff_buffer(hwloc_topology_diff_t diff, const char *refname, char **bufferp, int *buflenp) {
    return hwloc_nolibxml_export_diff_buffer(diff, refname, bufferp, buflenp);
}

static char *
hwloc__nolibxml_import_ignore_spaces(char *buffer)
{
  return wrap_hwloc__nolibxml_import_ignore_spaces(buffer);
}

static int
hwloc__nolibxml_import_next_attr(hwloc__xml_import_state_t state, char **namep, char **valuep)
{
  return wrap_hwloc__nolibxml_import_next_attr(state, namep, valuep);
}

static int
hwloc__nolibxml_import_find_child(hwloc__xml_import_state_t state,
				  hwloc__xml_import_state_t childstate,
				  char **tagp)
{
  return wrap_hwloc__nolibxml_import_find_child(state, childstate, tagp);
}

static int
hwloc__nolibxml_import_close_tag(hwloc__xml_import_state_t state)
{
  return wrap_hwloc__nolibxml_import_close_tag(state);
}

static void
hwloc__nolibxml_import_close_child(hwloc__xml_import_state_t state)
{
  wrap_hwloc__nolibxml_import_close_child(state);
}

static int
hwloc__nolibxml_import_get_content(hwloc__xml_import_state_t state,
				   const char **beginp, size_t expected_length)
{
  return wrap_hwloc__nolibxml_import_get_content(state, beginp, expected_length);
}

static void
hwloc__nolibxml_import_close_content(hwloc__xml_import_state_t state)
{
  wrap_hwloc__nolibxml_import_close_content(state);
}

static int
hwloc_nolibxml_look_init(struct hwloc_xml_backend_data_s *bdata,
			 struct hwloc__xml_import_state_s *state)
{
  return wrap_hwloc_nolibxml_look_init(bdata, state);
}

/* can be called at the end of the import (to cleanup things early),
 * or by backend_exit() if load failed for other reasons.
 */
static void
hwloc_nolibxml_free_buffers(struct hwloc_xml_backend_data_s *bdata)
{
  wrap_hwloc_nolibxml_free_buffers(bdata);
}

static void
hwloc_nolibxml_look_done(struct hwloc_xml_backend_data_s *bdata, int result)
{
  wrap_hwloc_nolibxml_look_done(bdata, result);
}

/********************
 * Backend routines *
 ********************/

static void
hwloc_nolibxml_backend_exit(struct hwloc_xml_backend_data_s *bdata)
{
  wrap_hwloc_nolibxml_backend_exit(bdata);
}

static int
hwloc_nolibxml_read_file(const char *xmlpath, char **bufferp, size_t *buflenp)
{
  return wrap_hwloc_nolibxml_read_file(xmlpath, bufferp, buflenp);
}

static int
hwloc_nolibxml_backend_init(struct hwloc_xml_backend_data_s *bdata,
			    const char *xmlpath, const char *xmlbuffer, int xmlbuflen)
{
  return wrap_hwloc_nolibxml_backend_init(bdata, xmlpath, xmlbuffer, xmlbuflen);
}

static int
hwloc_nolibxml_import_diff(struct hwloc__xml_import_state_s *state,
			   const char *xmlpath, const char *xmlbuffer, int xmlbuflen,
			   hwloc_topology_diff_t *firstdiffp, char **refnamep)
{
  return wrap_hwloc_nolibxml_import_diff(state, xmlpath, xmlbuffer, xmlbuflen, firstdiffp, refnamep);
}

/*******************
 * Export routines *
 *******************/

typedef struct hwloc__nolibxml_export_state_data_s {
  char *buffer; /* (moving) buffer where to write */
  size_t written; /* how many bytes were written (or would have be written if not truncated) */
  size_t remaining; /* how many bytes are still available in the buffer */
  unsigned indent; /* indentation level for the next line */
  unsigned nr_children;
  unsigned has_content;
} __hwloc_attribute_may_alias * hwloc__nolibxml_export_state_data_t;

static void
hwloc__nolibxml_export_update_buffer(hwloc__nolibxml_export_state_data_t ndata, int res)
{
  if (res >= 0) {
    ndata->written += res;
    if (res >= (int) ndata->remaining)
      res = ndata->remaining>0 ? (int)ndata->remaining-1 : 0;
    ndata->buffer += res;
    ndata->remaining -= res;
  }
}

static char *
hwloc__nolibxml_export_escape_string(const char *src)
{
  size_t fulllen, sublen;
  char *escaped, *dst;

  fulllen = strlen(src);

  sublen = strcspn(src, "\n\r\t\"<>&");
  if (sublen == fulllen)
    return NULL; /* nothing to escape */

  escaped = malloc(fulllen*6+1); /* escaped chars are replaced by at most 6 char */
  dst = escaped;

  memcpy(dst, src, sublen);
  src += sublen;
  dst += sublen;

  while (*src) {
    int replen;
    switch (*src) {
    case '\n': strcpy(dst, "&#10;");  replen=5; break;
    case '\r': strcpy(dst, "&#13;");  replen=5; break;
    case '\t': strcpy(dst, "&#9;");   replen=4; break;
    case '\"': strcpy(dst, "&quot;"); replen=6; break;
    case '<':  strcpy(dst, "&lt;");   replen=4; break;
    case '>':  strcpy(dst, "&gt;");   replen=4; break;
    case '&':  strcpy(dst, "&amp;");  replen=5; break;
    default: replen=0; break;
    }
    dst+=replen; src++;

    sublen = strcspn(src, "\n\r\t\"<>&");
    memcpy(dst, src, sublen);
    src += sublen;
    dst += sublen;
  }

  *dst = 0;
  return escaped;
}

static void
hwloc__nolibxml_export_new_child(hwloc__xml_export_state_t parentstate,
				 hwloc__xml_export_state_t state,
				 const char *name)
{
  hwloc__nolibxml_export_state_data_t npdata = (void *) parentstate->data;
  hwloc__nolibxml_export_state_data_t ndata = (void *) state->data;
  int res;

  assert(!npdata->has_content);
  if (!npdata->nr_children) {
    res = hwloc_snprintf(npdata->buffer, npdata->remaining, ">\n");
    hwloc__nolibxml_export_update_buffer(npdata, res);
  }
  npdata->nr_children++;

  state->parent = parentstate;
  state->new_child = parentstate->new_child;
  state->new_prop = parentstate->new_prop;
  state->add_content = parentstate->add_content;
  state->end_object = parentstate->end_object;
  state->global = parentstate->global;

  ndata->buffer = npdata->buffer;
  ndata->written = npdata->written;
  ndata->remaining = npdata->remaining;
  ndata->indent = npdata->indent + 2;

  ndata->nr_children = 0;
  ndata->has_content = 0;

  res = hwloc_snprintf(ndata->buffer, ndata->remaining, "%*s<%s", (int) npdata->indent, "", name);
  hwloc__nolibxml_export_update_buffer(ndata, res);
}

static void
hwloc__nolibxml_export_new_prop(hwloc__xml_export_state_t state, const char *name, const char *value)
{
  hwloc__nolibxml_export_state_data_t ndata = (void *) state->data;
  char *escaped = hwloc__nolibxml_export_escape_string(value);
  int res = hwloc_snprintf(ndata->buffer, ndata->remaining, " %s=\"%s\"", name, escaped ? (const char *) escaped : value);
  hwloc__nolibxml_export_update_buffer(ndata, res);
  free(escaped);
}

static void
hwloc__nolibxml_export_end_object(hwloc__xml_export_state_t state, const char *name)
{
  hwloc__nolibxml_export_state_data_t ndata = (void *) state->data;
  hwloc__nolibxml_export_state_data_t npdata = (void *) state->parent->data;
  int res;

  assert (!(ndata->has_content && ndata->nr_children));
  if (ndata->has_content) {
    res = hwloc_snprintf(ndata->buffer, ndata->remaining, "</%s>\n", name);
  } else if (ndata->nr_children) {
    res = hwloc_snprintf(ndata->buffer, ndata->remaining, "%*s</%s>\n", (int) npdata->indent, "", name);
  } else {
    res = hwloc_snprintf(ndata->buffer, ndata->remaining, "/>\n");
  }
  hwloc__nolibxml_export_update_buffer(ndata, res);

  npdata->buffer = ndata->buffer;
  npdata->written = ndata->written;
  npdata->remaining = ndata->remaining;
}

static void
hwloc__nolibxml_export_add_content(hwloc__xml_export_state_t state, const char *buffer, size_t length __hwloc_attribute_unused)
{
  hwloc__nolibxml_export_state_data_t ndata = (void *) state->data;
  int res;

  assert(!ndata->nr_children);
  if (!ndata->has_content) {
    res = hwloc_snprintf(ndata->buffer, ndata->remaining, ">");
    hwloc__nolibxml_export_update_buffer(ndata, res);
  }
  ndata->has_content = 1;

  res = hwloc_snprintf(ndata->buffer, ndata->remaining, "%s", buffer);
  hwloc__nolibxml_export_update_buffer(ndata, res);
}

static size_t
hwloc___nolibxml_prepare_export(hwloc_topology_t topology, struct hwloc__xml_export_data_s *edata,
				char *xmlbuffer, int buflen, unsigned long flags)
{
  struct hwloc__xml_export_state_s state, childstate;
  hwloc__nolibxml_export_state_data_t ndata = (void *) &state.data;
  int v1export = flags & HWLOC_TOPOLOGY_EXPORT_XML_FLAG_V1;
  int res;

  HWLOC_BUILD_ASSERT(sizeof(*ndata) <= sizeof(state.data));

  state.new_child = hwloc__nolibxml_export_new_child;
  state.new_prop = hwloc__nolibxml_export_new_prop;
  state.add_content = hwloc__nolibxml_export_add_content;
  state.end_object = hwloc__nolibxml_export_end_object;
  state.global = edata;

  ndata->indent = 0;
  ndata->written = 0;
  ndata->buffer = xmlbuffer;
  ndata->remaining = buflen;

  ndata->nr_children = 1; /* don't close a non-existing previous tag when opening the topology tag */
  ndata->has_content = 0;

  res = hwloc_snprintf(ndata->buffer, ndata->remaining,
		 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		 "<!DOCTYPE topology SYSTEM \"%s\">\n", v1export ? "hwloc.dtd" : "hwloc2.dtd");
  hwloc__nolibxml_export_update_buffer(ndata, res);
  hwloc__nolibxml_export_new_child(&state, &childstate, "topology");
  if (!(flags & HWLOC_TOPOLOGY_EXPORT_XML_FLAG_V1))
    hwloc__nolibxml_export_new_prop(&childstate, "version", "2.0");
  hwloc__xml_export_topology (&childstate, topology, flags);
  hwloc__nolibxml_export_end_object(&childstate, "topology");

  return ndata->written+1; /* ending \0 */
}

static int
hwloc_nolibxml_export_buffer(hwloc_topology_t topology, struct hwloc__xml_export_data_s *edata,
			     char **bufferp, int *buflenp, unsigned long flags)
{
  char *buffer;
  size_t bufferlen, res;

  bufferlen = 16384; /* random guess for large enough default */
  buffer = malloc(bufferlen);
  if (!buffer)
    return -1;
  res = hwloc___nolibxml_prepare_export(topology, edata, buffer, (int)bufferlen, flags);

  if (res > bufferlen) {
    char *tmp = realloc(buffer, res);
    if (!tmp) {
      free(buffer);
      return -1;
    }
    buffer = tmp;
    hwloc___nolibxml_prepare_export(topology, edata, buffer, (int)res, flags);
  }

  *bufferp = buffer;
  *buflenp = (int)res;
  return 0;
}

static int
hwloc_nolibxml_export_file(hwloc_topology_t topology, struct hwloc__xml_export_data_s *edata,
			   const char *filename, unsigned long flags)
{
  FILE *file;
  char *buffer;
  int bufferlen;
  int ret;

  ret = wrap_hwloc_nolibxml_export_buffer(topology, edata, &buffer, &bufferlen, flags);
  if (ret < 0)
    return -1;

  if (!strcmp(filename, "-")) {
    file = stdout;
  } else {
    file = fopen(filename, "w");
    if (!file) {
      free(buffer);
      return -1;
    }
  }

  ret = (int)fwrite(buffer, 1, bufferlen-1 /* don't write the ending \0 */, file);
  if (ret == bufferlen-1) {
    ret = 0;
  } else {
    errno = ferror(file);
    ret = -1;
  }

  free(buffer);

  if (file != stdout)
    fclose(file);
  return ret;
}

static size_t
hwloc___nolibxml_prepare_export_diff(hwloc_topology_diff_t diff, const char *refname, char *xmlbuffer, int buflen)
{
  struct hwloc__xml_export_state_s state, childstate;
  hwloc__nolibxml_export_state_data_t ndata = (void *) &state.data;
  int res;

  HWLOC_BUILD_ASSERT(sizeof(*ndata) <= sizeof(state.data));

  state.new_child = hwloc__nolibxml_export_new_child;
  state.new_prop = hwloc__nolibxml_export_new_prop;
  state.add_content = hwloc__nolibxml_export_add_content;
  state.end_object = hwloc__nolibxml_export_end_object;
  state.global = NULL;

  ndata->indent = 0;
  ndata->written = 0;
  ndata->buffer = xmlbuffer;
  ndata->remaining = buflen;

  ndata->nr_children = 1; /* don't close a non-existing previous tag when opening the topology tag */
  ndata->has_content = 0;

  res = hwloc_snprintf(ndata->buffer, ndata->remaining,
		 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		 "<!DOCTYPE topologydiff SYSTEM \"hwloc2-diff.dtd\">\n");
  hwloc__nolibxml_export_update_buffer(ndata, res);
  hwloc__nolibxml_export_new_child(&state, &childstate, "topologydiff");
  if (refname)
    hwloc__nolibxml_export_new_prop(&childstate, "refname", refname);
  hwloc__xml_export_diff (&childstate, diff);
  hwloc__nolibxml_export_end_object(&childstate, "topologydiff");

  return ndata->written+1;
}

static int
hwloc_nolibxml_export_diff_buffer(hwloc_topology_diff_t diff, const char *refname, char **bufferp, int *buflenp)
{
  char *buffer;
  size_t bufferlen, res;

  bufferlen = 16384; /* random guess for large enough default */
  buffer = malloc(bufferlen);
  if (!buffer)
    return -1;
  res = hwloc___nolibxml_prepare_export_diff(diff, refname, buffer, (int)bufferlen);

  if (res > bufferlen) {
    char *tmp = realloc(buffer, res);
    if (!tmp) {
      free(buffer);
      return -1;
    }
    buffer = tmp;
    hwloc___nolibxml_prepare_export_diff(diff, refname, buffer, (int)res);
  }

  *bufferp = buffer;
  *buflenp = (int)res;
  return 0;
}

static int
hwloc_nolibxml_export_diff_file(hwloc_topology_diff_t diff, const char *refname, const char *filename)
{
  FILE *file;
  char *buffer;
  int bufferlen;
  int ret;

  ret = wrap_hwloc_nolibxml_export_diff_buffer(diff, refname, &buffer, &bufferlen);
  if (ret < 0)
    return -1;

  if (!strcmp(filename, "-")) {
    file = stdout;
  } else {
    file = fopen(filename, "w");
    if (!file) {
      free(buffer);
      return -1;
    }
  }

  ret = (int)fwrite(buffer, 1, bufferlen-1 /* don't write the ending \0 */, file);
  if (ret == bufferlen-1) {
    ret = 0;
  } else {
    errno = ferror(file);
    ret = -1;
  }

  free(buffer);

  if (file != stdout)
    fclose(file);
  return ret;
}

static void
hwloc_nolibxml_free_buffer(void *xmlbuffer)
{
  wrap_hwloc_nolibxml_free_buffer(xmlbuffer);
}

/*************
 * Callbacks *
 *************/

static struct hwloc_xml_callbacks hwloc_xml_nolibxml_callbacks = {
  wrap_hwloc_nolibxml_backend_init,
  wrap_hwloc_nolibxml_export_file,
  wrap_hwloc_nolibxml_export_buffer,
  wrap_hwloc_nolibxml_free_buffer,
  wrap_hwloc_nolibxml_import_diff,
  wrap_hwloc_nolibxml_export_diff_file,
  wrap_hwloc_nolibxml_export_diff_buffer
};

static struct hwloc_xml_component hwloc_nolibxml_xml_component = {
  &hwloc_xml_nolibxml_callbacks,
  NULL
};

const struct hwloc_component hwloc_xml_nolibxml_component = {
  HWLOC_COMPONENT_ABI,
  NULL, NULL,
  HWLOC_COMPONENT_TYPE_XML,
  0,
  &hwloc_nolibxml_xml_component
};
