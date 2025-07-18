/*

  This file is provided under a dual BSD/GPLv2 license.  When using or
  redistributing this file, you may do so under either license.

  GPL LICENSE SUMMARY

  Copyright(c) 2016 Intel Corporation.

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  Contact Information:
  Intel Corporation, www.intel.com

  BSD LICENSE

  Copyright(c) 2016 Intel Corporation.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* Copyright (c) 2003-2016 Intel Corporation. All rights reserved. */

#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>		/* cpu_set */
#include <ctype.h>		/* isalpha */
#include <stdbool.h>

#include "psm_user.h"
#include "psm2_hal.h"
#include "psm_mq_internal.h"
#include "psm_am_internal.h"
#include "ips_proto_params.h"

#ifdef PSM_SOCKETS
#include <netdb.h>
#endif
/*
 * Endpoint management
 */
psm2_ep_t psm3_opened_endpoint = NULL;
int psm3_opened_endpoint_count = 0;
// list of network ids for all local devices
// We use this to determine if the psm2_nid_t() of a given epid matches a
// local device in which case we can use intra-node (ptl_am shm) comms
// with the given epid
// for OPA/IB this is the LID
// for IPv4 this is the full 32 bit IPv4 address
// for IPv6 this is the full 128 bit IPv6 address
static psmi_lock_t nids_lock;
static psm2_nid_t *hfi_nids;
static uint32_t nnids;	// number of populated entries in hfi_nids array

void psm3_ep_init(void)
{
	psmi_init_lock(&nids_lock);
}

void psm3_ep_fini(void)
{
	psmi_destroy_lock(&nids_lock);
}

static psm2_error_t psm3_ep_open_device(const psm2_ep_t ep,
				       const struct psm3_ep_open_opts *opts,
				       const psm2_uuid_t unique_job_key);
static void psm3_ep_close_device(const psm2_ep_t ep);

/*
 * Device management
 *
 * PSM uses "devices" as components to manage communication to self, to peers
 * reachable via shared memory and finally to peers reachable only through
 * hfi.
 */

psm2_error_t psm3_parse_devices(int devices[PTL_MAX_INIT]);
int psm3_device_is_enabled(const int devices[PTL_MAX_INIT], int devid);
int psm3_ep_device_is_enabled(const psm2_ep_t ep, int devid);

psm2_error_t psm3_ep_num_devunits(uint32_t *num_units_o)
{
	static int num_units = -1;

	PSM2_LOG_MSG("entering");

	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	if (num_units == -1) {
		num_units = psmi_hal_get_num_units();
		if (num_units == -1)
			num_units = 0;
	}

	*num_units_o = (uint32_t) num_units;
	PSM2_LOG_MSG("leaving");
	return PSM2_OK;
}

psm2_error_t psm3_ep_query(int *num_of_epinfo, psm2_epinfo_t *array_of_epinfo)
{
	psm2_error_t err = PSM2_OK;
	int i;
	psm2_ep_t ep;

	PSM2_LOG_MSG("entering");
	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	if (*num_of_epinfo <= 0) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid psm3_ep_query parameters");
		PSM2_LOG_MSG("leaving");
		return err;
	}

	if (psm3_opened_endpoint == NULL) {
		err = psm3_handle_error(NULL, PSM2_EP_WAS_CLOSED,
					"PSM Endpoint is closed or does not exist");
		PSM2_LOG_MSG("leaving");
		return err;
	}

	ep = psm3_opened_endpoint;
	for (i = 0; i < *num_of_epinfo; i++) {
		if (ep == NULL)
			break;
		array_of_epinfo[i].ep = ep;
		array_of_epinfo[i].epid = ep->epid;
		array_of_epinfo[i].jkey = ep->jkey;
		memcpy(array_of_epinfo[i].uuid,
		       (void *)ep->uuid, sizeof(psm2_uuid_t));
		uuid_unparse_lower(ep->uuid, array_of_epinfo[i].uuid_str);
		ep = ep->user_ep_next;
	}
	*num_of_epinfo = i;
	PSM2_LOG_MSG("leaving");
	return err;
}

psm2_error_t psm3_ep_epid_lookup(psm2_epid_t epid, psm2_epconn_t *epconn)
{
	psm2_error_t err = PSM2_OK;
	psm2_epaddr_t epaddr;
	psm2_ep_t ep;

	PSM2_LOG_MSG("entering");
	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	/* Need to have an opened endpoint before we can resolve epids */
	if (psm3_opened_endpoint == NULL) {
		err = psm3_handle_error(NULL, PSM2_EP_WAS_CLOSED,
					"PSM Endpoint is closed or does not exist");
		PSM2_LOG_MSG("leaving");
		return err;
	}

	ep = psm3_opened_endpoint;
	while (ep) {
		epaddr = psm3_epid_lookup(ep, epid);
		if (!epaddr) {
			ep = ep->user_ep_next;
			continue;
		}

		/* Found connection for epid. Return info about endpoint to caller. */
		psmi_assert_always(epaddr->ptlctl->ep == ep);
		epconn->addr = epaddr;
		epconn->ep = ep;
		epconn->mq = ep->mq;
		PSM2_LOG_MSG("leaving");
		return err;
	}

	err = psm3_handle_error(NULL, PSM2_EPID_UNKNOWN,
				"Endpoint connection status unknown");
	PSM2_LOG_MSG("leaving");
	return err;
}

psm2_error_t psm3_ep_epid_lookup2(psm2_ep_t ep, psm2_epid_t epid, psm2_epconn_t *epconn)
{
	psm2_error_t err = PSM2_OK;

	PSM2_LOG_MSG("entering");
	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	/* Need to have an opened endpoint before we can resolve epids */
	if (ep == NULL) {
		err = psm3_handle_error(NULL, PSM2_EP_WAS_CLOSED,
					"PSM Endpoint is closed or does not exist");
		PSM2_LOG_MSG("leaving");
		return err;
	}

	if (epconn == NULL) {
		err = psm3_handle_error(ep, PSM2_PARAM_ERR,
					"Invalid output parameter");
		PSM2_LOG_MSG("leaving");
		return err;
	}

	psm2_epaddr_t epaddr = psm3_epid_lookup(ep, epid);
	if (epaddr) {
		/* Found connection for epid. Return info about endpoint to caller. */
		psmi_assert_always(epaddr->ptlctl->ep == ep);
		epconn->addr = epaddr;
		epconn->ep = ep;
		epconn->mq = ep->mq;
		PSM2_LOG_MSG("leaving");
		return err;
	}

	err = psm3_handle_error(ep, PSM2_EPID_UNKNOWN,
				"Endpoint connection status unknown");
	PSM2_LOG_MSG("leaving");
	return err;
}

psm2_error_t psm3_epaddr_to_epid(psm2_epaddr_t epaddr, psm2_epid_t *epid)
{
	psm2_error_t err = PSM2_OK;
	PSM2_LOG_MSG("entering");
	if (epaddr && epid) {
		*epid = epaddr->epid;
	}
	else {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid input epaddr or output epid parameter");
	}
	PSM2_LOG_MSG("leaving");
	return err;
}

static psm2_error_t devnids_init(void)
{
	uint32_t num_units = 0;
	int i;
	psm2_error_t err = PSM2_OK;

	PSMI_LOCK(nids_lock);

	if (hfi_nids == NULL) {
		if ((err = psm3_ep_num_devunits(&num_units)))
			goto fail;
		hfi_nids = (psm2_nid_t *)
		    psmi_calloc(PSMI_EP_NONE, UNDEFINED,
				num_units * psmi_hal_get_num_ports()*psm3_addr_per_nic, sizeof(*hfi_nids));
		if (hfi_nids == NULL) {
			err = psm3_handle_error(NULL, PSM2_NO_MEMORY,
						"Couldn't allocate memory for dev_nids structure");
			goto fail;
		}

		for (i = 0; i < num_units; i++) {
			int j;
			for (j = PSM3_NIC_MIN_PORT; j <= PSM3_NIC_MAX_PORT; j++) {
				int k;
				for (k = 0; k < psm3_addr_per_nic; k++) {
					int lid = psmi_hal_get_port_lid(i, j, k);
					int ret, idx = 0;
					psmi_subnet128_t subnet = { };
					psmi_naddr128_t addr = { };
					psmi_gid128_t gid = { };

					// skip ports which aren't ready for use
					if (lid <= 0)
						continue;
					ret = psmi_hal_get_port_subnet(i, j, k, &subnet, &addr, &idx, &gid);
					if (ret == -1)
						continue;
					hfi_nids[nnids] = psm3_build_nid(i, addr, lid);
					_HFI_VDBG("NIC unit %d, port %d addr_index %d, found %s "
						  "GID[%d] %s subnet %s\n",
						i, j, k,
						psm3_nid_fmt(hfi_nids[nnids], 0),
						idx, psm3_gid128_fmt(gid, 1),
						psm3_subnet128_fmt(subnet, 2));
					nnids++;
				}
			}
		}
		if (nnids == 0) {
			err = psm3_handle_error(NULL, PSM2_EP_DEVICE_FAILURE,
						"Couldn't find any unfiltered units");
			goto fail;
		}
	}

fail:
	PSMI_UNLOCK(nids_lock);
	return err;
}

static void devnids_destroy(void)
{
	PSMI_LOCK(nids_lock);

	if (hfi_nids) {
		psmi_free(hfi_nids);
		hfi_nids = NULL;
		nnids = 0;
	}

	PSMI_UNLOCK(nids_lock);
}

// This is used to find devices with the same address as another process,
// implying intra-node comms.
// We populate hfi_nids and nnids with the set of network ids (NID) for
// all the local NICs.
// The caller will see if any of these NIDs match the NID of the remote process.
// Note that NIDs are globally unique and include both subnet and NIC address
// information, so we can compare them regardless of their subnet.
// NIDs which are not on the same subnet will not match.
// NIDs on the same subnet only match if they are the same NIC.
// Two local NICs with the same subnet and same address is an unexpected
// invalid config, and will silently match the two NICs.
//
static psm2_error_t
psm3_ep_devnids(psm2_nid_t **nids, uint32_t *num_nids_o)
{
	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	psm2_error_t err = devnids_init();
	if (err != PSM2_OK)
		return err;

	*nids = hfi_nids;
	*num_nids_o = nnids;

	return PSM2_OK;
}

// Indicate if the given epid is a local process.
// In which case we can use intra-node shared memory comms with it.
psm2_error_t
psm3_ep_epid_share_memory(psm2_ep_t ep, psm2_epid_t epid, int *result_o)
{
	int result = 0;
	uint32_t num_nids = 0;
	psm2_nid_t nid;
	psm2_nid_t *nids = NULL;
	int i;
	psm2_error_t err;

	PSM2_LOG_MSG("entering");
	psmi_assert_always(ep != NULL);
	PSMI_ERR_UNLESS_INITIALIZED(ep);

	if ((!psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS)) ||
		(psm3_epid_addr_fmt(epid) == PSMI_ADDR_FMT_SHM)) {
		// FMT_SHM has a NID based on 1st local NIC
		if (0 == psm3_nid_cmp_internal(psm3_epid_nid(ep->epid), psm3_epid_nid(epid)))
			result = 1;	// same node
		else
			_HFI_ERROR("attempting to run multi-node job without 'nic' in PSM3_DEVICES: remote process %s NIC %s doesn't match local NIC %s\n",
					psm3_epid_fmt_internal(epid, 0),
					psm3_epid_fmt_addr(epid, 1),
					psm3_epid_fmt_addr(ep->epid, 2));
	} else {
		nid = psm3_epid_nid(epid);
		err = psm3_ep_devnids(&nids, &num_nids);
		if (err) {
			PSM2_LOG_MSG("leaving");
			return err;
		}
		for (i = 0; i < num_nids; i++) {
			if (0 == psm3_nid_cmp_internal(nid, nids[i])) {
				/* we share memory if the nid is the same. */
				result = 1;
				_HFI_VDBG("remote process %s NIC %s matches local NIC %s\n",
					psm3_epid_fmt_internal(epid, 0),
					psm3_epid_fmt_addr(epid, 1),
					psm3_nid_fmt(nids[i], 2));
				break;
			}
		}
	}
	*result_o = result;
	PSM2_LOG_MSG("leaving");
	return PSM2_OK;
}

psm2_error_t psm3_ep_open_opts_get_defaults(struct psm3_ep_open_opts *opts)
{
	PSM2_LOG_MSG("entering");

	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	if (!opts)
		return PSM2_PARAM_ERR;

	/* Set in order in the structure. */
	opts->timeout = 30000000000LL;	/* 30 sec */
	opts->unit = PSM3_NIC_ANY;
	opts->affinity = PSM2_EP_OPEN_AFFINITY_SET;
	opts->shm_mbytes = 0;	/* deprecated in psm2.h */
	opts->sendbufs_num = 1024;
	opts->network_pkey = psmi_hal_get_default_pkey();
	opts->port = PSM3_NIC_PORT_ANY;
	opts->addr_index = PSM3_ADDR_INDEX_ANY;
	opts->outsl = PSMI_SL_DEFAULT;
	opts->service_id = HFI_DEFAULT_SERVICE_ID;
	opts->path_res_type = PSM2_PATH_RES_NONE;
	opts->senddesc_num = 4096;
	opts->imm_size = 0;

	psmi_hal_ep_open_opts_get_defaults(opts);

	PSM2_LOG_MSG("leaving");
	return PSM2_OK;
}

psm2_error_t psm3_poll_noop(ptl_t *ptl, int replyonly, bool force);

// open a single internal EP for a single NIC
// For 1st internal EP opts may indicate PSM3_NIC_ANY in which case
// psm3_ep_open_device will let psm3_context_open pick the NIC based on
// PSM3_NIC_SELECTION_ALG.
// For multirail and when opening additional QPs for the NIC, opts will
// select a specific NIC.
psm2_error_t
psm3_ep_open_internal(psm2_uuid_t const unique_job_key, int *devid_enabled,
		       struct psm3_ep_open_opts const *opts_i, psm2_mq_t mq,
		       psm2_ep_t *epo, psm2_epid_t *epido)
{
	psm2_ep_t ep = NULL;
	uint32_t num_units = 0;
	size_t len;
	psm2_error_t err;
	psm2_epaddr_t epaddr = NULL;
	char buf[128], *p;
	union psmi_envvar_val envvar_val;
	size_t ptl_sizes;
	struct psm3_ep_open_opts opts;
	ptl_t *amsh_ptl, *ips_ptl, *self_ptl;
	int i;

	/* First get the set of default options, we overwrite with the user's
	 * desired values afterwards */
	if ((err = psm3_ep_open_opts_get_defaults(&opts)))
		goto fail;

	if (opts_i->timeout != -1)
		opts.timeout = opts_i->timeout;
	if (opts_i->unit != -1)
		opts.unit = opts_i->unit;
	if (opts_i->affinity != -1)
		opts.affinity = opts_i->affinity;
	if (opts_i->sendbufs_num != -1)
		opts.sendbufs_num = opts_i->sendbufs_num;
	if (opts_i->network_pkey != psmi_hal_get_default_pkey())
		opts.network_pkey = opts_i->network_pkey;
	if (opts_i->port != 0)
		opts.port = opts_i->port;
	if (opts_i->addr_index != -1)
		opts.addr_index = opts_i->addr_index;
	if (opts_i->outsl != -1)
		opts.outsl = opts_i->outsl;
	if (opts_i->service_id)
		opts.service_id = (uint64_t) opts_i->service_id;
#ifdef PSM3_PATH_REC_QUERY
	if (opts_i->path_res_type != PSM2_PATH_RES_NONE)
		opts.path_res_type = opts_i->path_res_type;
#endif
	if (opts_i->senddesc_num)
		opts.senddesc_num = opts_i->senddesc_num;
	if (opts_i->imm_size)
		opts.imm_size = opts_i->imm_size;

	/* Get Service ID from environment */
	if (!psm3_getenv("PSM3_IB_SERVICE_ID",
			 "Service ID for RV module RC QP connection establishment",
			 PSMI_ENVVAR_LEVEL_USER,
			 PSMI_ENVVAR_TYPE_ULONG_FLAGS, // FLAGS only affects output: hex
			 (union psmi_envvar_val)HFI_DEFAULT_SERVICE_ID,
			 &envvar_val)) {
		opts.service_id = (uint64_t) envvar_val.e_ulonglong;
	}

#ifdef PSM3_PATH_REC_QUERY
	const char *PSM3_PATH_REC_HELP =
			 "Mechanism to query NIC path record [opp, umad or none] (default is none)";
	/* Get Path resolution type from environment Possible choices are:
	 *
	 * NONE : Default same as previous instances. Utilizes static data.
	 * OPP  : Use OFED Plus Plus library to do path record queries.
	 * UMAD : Use raw libibumad interface to form and process path records.
	 */
	if (!psm3_getenv("PSM3_PATH_REC", PSM3_PATH_REC_HELP,
			 PSMI_ENVVAR_LEVEL_USER, PSMI_ENVVAR_TYPE_STR,
			 (union psmi_envvar_val)"none", &envvar_val)) {
		if (!strcasecmp(envvar_val.e_str, "none"))
			opts.path_res_type = PSM2_PATH_RES_NONE;
		else if (!strcasecmp(envvar_val.e_str, "opp"))
			opts.path_res_type = PSM2_PATH_RES_OPP;
		else if (!strcasecmp(envvar_val.e_str, "umad"))
			opts.path_res_type = PSM2_PATH_RES_UMAD;
		else {
			_HFI_INFO("Invalid value for PSM3_PATH_REC ('%s') %-40s Using: none\n",
				envvar_val.e_str, PSM3_PATH_REC_HELP);
			opts.path_res_type = PSM2_PATH_RES_NONE;
		}
	}
#else
	opts.path_res_type = PSM2_PATH_RES_NONE;
#endif

	/* Get user specified port number to use. */
	if (!psm3_getenv("PSM3_NIC_PORT", "NIC Port number (0 autodetects)",
			 PSMI_ENVVAR_LEVEL_HIDDEN, PSMI_ENVVAR_TYPE_LONG,
			 (union psmi_envvar_val)PSM3_NIC_PORT_ANY,
			 &envvar_val)) {
		opts.port = envvar_val.e_long;
	}

	/* Get service level from environment, path-query overrides it */
	if (!psm3_getenv
	    ("PSM3_NIC_SL", "NIC outging ServiceLevel number (default 0)",
	     PSMI_ENVVAR_LEVEL_HIDDEN,
	     PSMI_ENVVAR_TYPE_LONG,
	     (union psmi_envvar_val)PSMI_SL_DEFAULT, &envvar_val)) {
		opts.outsl = envvar_val.e_long;
	}

	/* Get network key from environment. MVAPICH and other vendor MPIs do not
	 * specify it on ep open and we may require it for vFabrics.
	 * path-query will override it.
	 */
	if (!psm3_getenv("PSM3_PKEY",
			 "PKey to use for endpoint (0=use slot 0)",
			 PSMI_ENVVAR_LEVEL_HIDDEN,
			 PSMI_ENVVAR_TYPE_ULONG_FLAGS,	// show in hex
			 (union psmi_envvar_val)((unsigned int)(psmi_hal_get_default_pkey())),
			 &envvar_val)) {
		opts.network_pkey = (uint64_t) envvar_val.e_ulong;
	}

	/* BACKWARDS COMPATIBILITY:  Open MPI likes to choose its own PKEY of
	   0x7FFF.  That's no longer a valid default, so override it if the
	   client was compiled against PSM v1 */
	if (PSMI_VERNO_GET_MAJOR(psm3_verno_client()) < 2 &&
			opts.network_pkey == 0x7FFF) {
		opts.network_pkey = psmi_hal_get_default_pkey();;
	}

	/* Get number of default send buffers from environment */
	if (!psm3_getenv("PSM3_NUM_SEND_BUFFERS",
			 "Number of send buffers to allocate [1024]",
			 PSMI_ENVVAR_LEVEL_HIDDEN,
			 PSMI_ENVVAR_TYPE_UINT,
			 (union psmi_envvar_val)1024, &envvar_val)) {
		opts.sendbufs_num = envvar_val.e_uint;
	}

	/* Get immediate data size - transfers less than immediate data size do
	 * not consume a send buffer and require just a send descriptor.
	 */
	if (!psm3_getenv_range("PSM3_SEND_IMMEDIATE_SIZE",
			 "Immediate data send size not requiring a buffer. Default 128.",
			 "Actual permitted upper limit is NIC dependent.",
			 PSMI_ENVVAR_LEVEL_HIDDEN, PSMI_ENVVAR_TYPE_UINT,
			 (union psmi_envvar_val)128,
			 (union psmi_envvar_val)0, (union psmi_envvar_val)1024,
			 NULL, NULL, &envvar_val)) {
		opts.imm_size = envvar_val.e_uint;
	}

	/* Get number of send descriptors - by default this is 4 times the number
	 * of send buffers - mainly used for short/inlined messages.
	 */
	if (!psm3_getenv("PSM3_NUM_SEND_DESCRIPTORS",
			 "Number of send descriptors to allocate [4096]",
			 PSMI_ENVVAR_LEVEL_HIDDEN,
			 PSMI_ENVVAR_TYPE_UINT,
			 (union psmi_envvar_val)4096, &envvar_val)) {
		opts.senddesc_num = envvar_val.e_uint;

		if (!PSMI_POWEROFTWO(opts.senddesc_num) ||
			  opts.senddesc_num < 256) {
			err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
			      "PSM3_NUM_SEND_DESCRIPTORS (%u)"
			      " should be a power of 2 greater than "
			      "or equal to 256\n", opts.senddesc_num);
			_HFI_ERROR( "PSM3_NUM_SEND_DESCRIPTORS (%u)"
                              " should be a power of 2 greater than "
                              "or equal to 256\n", opts.senddesc_num);
			goto fail;
		}
	}
	if (psm3_device_is_enabled(devid_enabled, PTL_DEVID_IPS)) {
		if ((err = psm3_ep_num_devunits(&num_units)) != PSM2_OK)
			goto fail;
	}

	/* do some error checking */
	if (opts.timeout < -1) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid timeout value %lld",
					(long long)opts.timeout);
		goto fail;
	} else if (num_units && (opts.unit < -1 || opts.unit >= (int)num_units)) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid Device Unit ID %d (%d units found)",
					opts.unit, num_units);
		goto fail;
	} else if ((opts.port < PSM3_NIC_MIN_PORT || opts.port > PSM3_NIC_MAX_PORT) &&
				opts.port != PSM3_NIC_PORT_ANY) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid Device port number %d",
					opts.port);
		goto fail;
	} else if ((opts.addr_index < -1 || opts.addr_index >= (int)psm3_addr_per_nic)) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid Device Address Index %d (%d addrs configured)",
					opts.addr_index, psm3_addr_per_nic);
		goto fail;
	} else if (opts.affinity < 0
		   || opts.affinity > PSM2_EP_OPEN_AFFINITY_FORCE) {
		err =
		    psm3_handle_error(NULL, PSM2_PARAM_ERR,
				      "Invalid Affinity option: %d",
				      opts.affinity);
		goto fail;
	} else if (opts.outsl < PSMI_SL_MIN || opts.outsl > PSMI_SL_MAX) {
		err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
					"Invalid SL number: %lld",
					(unsigned long long)opts.outsl);
		goto fail;
	}

	/* Allocate end point structure storage */
	ptl_sizes =
	    (psm3_device_is_enabled(devid_enabled, PTL_DEVID_SELF) ?
	     psm3_ptl_self.sizeof_ptl() : 0) +
	    (psm3_device_is_enabled(devid_enabled, PTL_DEVID_IPS) ?
	     psm3_ptl_ips.sizeof_ptl() : 0) +
	    (psm3_device_is_enabled(devid_enabled, PTL_DEVID_AMSH) ?
	     psm3_ptl_amsh.sizeof_ptl() : 0);
	if (ptl_sizes == 0)
		return PSM2_EP_NO_DEVICE;

	ep = (psm2_ep_t) psmi_memalign(PSMI_EP_NONE, UNDEFINED, 64,
				      sizeof(struct psm2_ep) + ptl_sizes);
	epaddr = (psm2_epaddr_t) psmi_calloc(PSMI_EP_NONE, PER_PEER_ENDPOINT,
					    1, sizeof(struct psm2_epaddr));
	if (ep == NULL || epaddr == NULL) {
		err = psm3_handle_error(NULL, PSM2_NO_MEMORY,
					"Couldn't allocate memory for %s structure",
					ep == NULL ? "psm2_ep" : "psm2_epaddr");
		goto fail;
	}
	memset(ep, 0, sizeof(struct psm2_ep) + ptl_sizes);

	/* Copy PTL enabled status */
	for (i = 0; i < PTL_MAX_INIT; i++)
		ep->devid_enabled[i] = devid_enabled[i];

	/* Matched Queue initialization.  We do this early because we have to
	 * make sure ep->mq exists and is valid before calling ips_do_work.
	 */
	ep->mq = mq;

	/* Get ready for PTL initialization */
	memcpy(&ep->uuid, (void *)unique_job_key, sizeof(psm2_uuid_t));
	ep->epaddr = epaddr;
	ep->memmode = mq->memmode;
	ep->hfi_num_sendbufs = opts.sendbufs_num;
	ep->service_id = opts.service_id;
	ep->path_res_type = opts.path_res_type;
	ep->hfi_num_descriptors = opts.senddesc_num;
	ep->hfi_imm_size = opts.imm_size;
	ep->errh = psm3_errhandler_global;	/* by default use the global one */
	ep->ptl_amsh.ep_poll = psm3_poll_noop;
	ep->ptl_ips.ep_poll = psm3_poll_noop;
	ep->connections = 0;

	// If supported and enabled, HAL will set these in context_open
	ep->rdmamode = 0;
#ifdef PSM_HAVE_REG_MR
	ep->mr_cache_mode = MR_CACHE_MODE_NONE;
	ep->hfi_num_send_rdma = 0;
#endif
#ifdef PSM_HAVE_RNDV_MOD
#ifdef PSM_HAVE_GPU
	ep->rv_gpu_cache_size = 0;
#endif /* PSM_HAVE_GPU */
#endif /* PSM_HAVE_RNDV_MOD */

	/* See how many iterations we want to spin before yielding */
	psm3_getenv("PSM3_YIELD_SPIN_COUNT",
		    "Spin poll iterations before yield",
		    PSMI_ENVVAR_LEVEL_HIDDEN,
		    PSMI_ENVVAR_TYPE_UINT,
		    (union psmi_envvar_val)PSMI_BLOCKUNTIL_POLLS_BEFORE_YIELD,
		    &envvar_val);
	ep->yield_spin_cnt = envvar_val.e_uint;

	/* Set skip_affinity flag if PSM is not allowed to set affinity */
	if (opts.affinity == PSM2_EP_OPEN_AFFINITY_SKIP)
		ep->skip_affinity = true;

	ptl_sizes = 0;
	amsh_ptl = ips_ptl = self_ptl = NULL;
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_AMSH)) {
		amsh_ptl = (ptl_t *) (ep->ptl_base_data + ptl_sizes);
		ptl_sizes += psm3_ptl_amsh.sizeof_ptl();
	}
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS)) {
		ips_ptl = (ptl_t *) (ep->ptl_base_data + ptl_sizes);
		ptl_sizes += psm3_ptl_ips.sizeof_ptl();
	}
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_SELF)) {
		self_ptl = (ptl_t *) (ep->ptl_base_data + ptl_sizes);
		ptl_sizes += psm3_ptl_self.sizeof_ptl();
	}

	// HFI Interface.
	if ((err = psm3_ep_open_device(ep, &opts, unique_job_key)))
		goto fail;
	psmi_assert_always(!psm3_epid_zero_internal(ep->epid));
	ep->epaddr->epid = ep->epid;

	_HFI_VDBG("psm3_ep_open_device() passed\n");

	/* Set our new label as soon as we know what it is */
	strncpy(buf, psm3_gethostname(), sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	p = buf + strlen(buf);

	/* If our rank is set, use it (same as mylabel). If not, use context */
	/* hostname.rank# or hostname.# (context), or hostname.pid#
	 */
	if (psm3_get_myrank() >= 0)
		len = snprintf(p, sizeof(buf) - strlen(buf), ":rank%d.", psm3_get_myrank());
	else
		len = snprintf(p, sizeof(buf) - strlen(buf), ":%s.",
				psm3_epid_fmt_context(ep->epid, 0));
	*(p + len) = '\0';
	ep->context_mylabel = psmi_strdup(ep, buf);
	if (ep->context_mylabel == NULL) {
		err = PSM2_NO_MEMORY;
		goto close;
	}
	/* psm3_set_mylabel(ep->context_mylabel); */

	if ((err = psm3_epid_set_hostname(psm3_epid_nid(ep->epid), buf, 0)))
		goto close;

	if (! mq->ep)	// only call on 1st EP within MQ
		psm3_mq_initstats(mq, ep->epid);

	PSM3_GPU_VERIFY_CAPABILITIES();

	_HFI_VDBG("start ptl device init...\n");
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_SELF)) {
		if ((err = psm3_ptl_self.init(ep, self_ptl, &ep->ptl_self)))
			goto close;
	}
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS)) {
		if ((err = psm3_ptl_ips.init(ep, ips_ptl, &ep->ptl_ips)))
			goto close;
	}
	/* If we're shm-only, this device is enabled above */
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_AMSH)) {
		if ((err = psm3_ptl_amsh.init(ep, amsh_ptl, &ep->ptl_amsh)))
			goto close;
	} else {
		/* We may have pre-attached as part of getting our rank for enabling
		 * shared contexts.  */
	}
	_HFI_VDBG("finish ptl device init...\n");

	/*
	 * Keep only IPS since only IPS support multi-rail, other devices
	 * are only setup once. IPS device can come to this function again.
	 */
	for (i = 0; i < PTL_MAX_INIT; i++) {
		if (devid_enabled[i] != PTL_DEVID_IPS) {
			devid_enabled[i] = -1;
		}
	}

	*epido = ep->epid;
	*epo = ep;

	return PSM2_OK;

close:
	psm3_ep_close_device(ep);
fail:
	if (ep != NULL) {
		// TBD need to cleanup ptl.fini and other details
		// see psm3_ep_close
		psmi_free(ep);
	}
	if (epaddr != NULL)
		psmi_free(epaddr);
	return err;
}

psm2_error_t
psm3_ep_open(psm2_uuid_t const unique_job_key,
	      struct psm3_ep_open_opts const *opts_i, psm2_ep_t *epo,
	      psm2_epid_t *epido)
{
	psm2_error_t err;
	psm2_mq_t mq;
	psm2_epid_t epid;
	psm2_ep_t ep, tmp;
	int i;
	int devid_enabled[PTL_MAX_INIT];
	struct psm3_ep_open_opts opts = *opts_i;
	struct multirail_config multirail_config = { 0 };

	PSM2_LOG_MSG("entering");
	PSMI_ERR_UNLESS_INITIALIZED(NULL);

	if (!epo || !epido)
		return PSM2_PARAM_ERR;

	psm3_print_rank_identify();

	/* Allowing only one EP (unless explicitly enabled). */
	if (psm3_opened_endpoint_count > 0 && !psm3_multi_ep_enabled) {
		PSM2_LOG_MSG("leaving");
		return PSM2_TOO_MANY_ENDPOINTS;
	}

	PSM3_GPU_EP_OPEN();

	/* Matched Queue initialization.  We do this early because we have to
	 * make sure ep->mq exists and is valid before calling ips_do_work.
	 */
	err = psm3_mq_malloc(&mq);
	PSMI_LOCK(psm3_creation_lock);
	if (err != PSM2_OK)
		goto fail;

	/* Set some of the MQ thresholds from the environment.
	   Do this before ptl initialization - the ptl may have other
	   constraints that will limit the MQ's settings. */
	err = psm3_mq_initialize_params(mq);
	if (err != PSM2_OK)
		goto fail;

	psmi_init_lock(&(mq->progress_lock));

	if ((err = psm3_parse_devices(devid_enabled)))
		goto fail;

	if (psm3_device_is_enabled(devid_enabled, PTL_DEVID_IPS)) {
		err = psm3_ep_multirail(&multirail_config);
		if (err != PSM2_OK)
			goto fail;

		/* If multi-rail is used, set the first ep unit/port */
		if (multirail_config.num_rails > 0) {
			opts.unit = multirail_config.units[0];
			opts.port = multirail_config.ports[0];
			opts.addr_index = multirail_config.addr_indexes[0];
		}
	}
#ifdef PSM_HAVE_GPU
	// if HAL doesn't support GDR Copy, it may disable Gdr Copy
	// by zeroing psm3_gpu_is_gdr_copy_enabled,
	// psm3_gpu_gdr_copy_limit_send, and
	// psm3_gpu_gdr_copy_limit_recv during gdr_open
	if (PSM3_GPU_IS_GDR_COPY_ENABLED)
		psmi_hal_gdr_open();
#endif

	err = psm3_ep_open_internal(unique_job_key,
				     devid_enabled, &opts, mq, &ep, &epid);
	if (err != PSM2_OK)
		goto fail;

	// list of all application opened EPs (multi-EP)
	if (psm3_opened_endpoint == NULL) {
		psm3_opened_endpoint = ep;
	} else {
		tmp = psm3_opened_endpoint;
		while (tmp->user_ep_next)
			tmp = tmp->user_ep_next;
		tmp->user_ep_next = ep;
	}
	psm3_opened_endpoint_count++;

	// list of all rails/QPs within this ep, this ep is the "master"
	ep->mctxt_prev = ep->mctxt_next = ep;
	ep->mctxt_master = ep;
	mq->ep = ep;

	psm3_print_ep_identify(ep);

	/* Active Message initialization */
	err = psm3_am_init_internal(ep);
	if (err != PSM2_OK)
		goto fail;

	*epo = ep;
	*epido = epid;

	if (psm3_device_is_enabled(devid_enabled, PTL_DEVID_IPS)) {
		int j;

		psmi_hal_context_initstats(ep);
		union psmi_envvar_val envvar_val;

		if (multirail_config.num_rails <= 0) {
			// the NIC has now been selected for our process
			// use the same NIC for any additional QPs below
			multirail_config.num_rails = 1;
			multirail_config.units[0] = ep->unit_id;
			multirail_config.ports[0] = ep->portnum;
			multirail_config.addr_indexes[0] = ep->addr_index;
		}
		// When QP_PER_NIC >1, creates more than 1 QP on each NIC and then
		// uses the multi-rail algorithms to spread the traffic across QPs
		// This helps get better BW when there are relatively few processes/node
		// care must be taken when combining this with user space RC QPs as
		// scalability (memory footprint) issues can be multiplied
		// This approach duplicates some per NIC resources (CQs, etc) but
		// provides a simple approach
		psm3_getenv("PSM3_QP_PER_NIC",
			"Number of sets of QPs to open per NIC [1]",
			PSMI_ENVVAR_LEVEL_USER,
			PSMI_ENVVAR_TYPE_UINT,
			(union psmi_envvar_val)1, &envvar_val);

		if ((multirail_config.num_rails * envvar_val.e_uint) > PSMI_MAX_QPS) {
			err = psm3_handle_error(NULL, PSM2_TOO_MANY_ENDPOINTS,
				"PSM3_QP_PER_NIC (%u) * num_rails (%d) > Max Support QPs (%u)",
				envvar_val.e_uint, multirail_config.num_rails, PSMI_MAX_QPS);
			goto fail;
		}

		for (j= 0; j< envvar_val.e_uint; j++) {
			// loop will open additional internal EPs for all
			// the additional QPs on 1st rail and for all the
			// additional rails and all the QPs on those rails
			for (i = 0; i < multirail_config.num_rails; i++) {
				_HFI_VDBG("rail %d unit %u port %u addr_index %d\n", i,
							multirail_config.units[i],
							multirail_config.ports[i],
							multirail_config.addr_indexes[i]);
				// did 0, 0 already above
				if (i == 0 && j== 0)
					continue;
				opts.unit = multirail_config.units[i];
				opts.port = multirail_config.ports[i];
				opts.addr_index = multirail_config.addr_indexes[i];

				/* Create secondary EP */
				err = psm3_ep_open_internal(unique_job_key,
						     devid_enabled, &opts, mq,
						     &tmp, &epid);
				if (err)
					goto fail;

				/* Point back to shared resources on the master EP */
				tmp->am_htable = ep->am_htable;

				/* Link secondary EP after master EP. */
				PSM_MCTXT_APPEND(ep, tmp);
				if (j == 0)
					psm3_print_ep_identify(tmp);
				psmi_hal_context_initstats(tmp);
			}
		}
	}

	_HFI_VDBG("psm3_ep_open() OK....\n");

fail:
	fflush(stdout);
	PSMI_UNLOCK(psm3_creation_lock);
	if (psm3_opened_endpoint_count == 0)
		PSM3_GPU_EP_CLOSE();
	PSM2_LOG_MSG("leaving");
	return err;
}

psm2_error_t psm3_ep_close(psm2_ep_t ep, int mode, int64_t timeout_in)
{
	psm2_error_t err = PSM2_OK;

	psm3_stats_ep_close();	// allow output of stats on 1st ep close if desired

#if _HFI_DEBUGGING
	uint64_t t_start = 0;
	if (_HFI_PRDBG_ON) {
		t_start = get_cycles();
	}
#endif

#ifdef PSM_HAVE_GPU
	/*
	 * The close on the gdr fd needs to be called before the
	 * close on the hfi fd as the the gdr device will hold
	 * reference count on the hfi device which will make the close
	 * on the hfi fd return without actually closing the fd.
	 */
	if (PSM3_GPU_IS_GDR_COPY_ENABLED)
		psmi_hal_gdr_close();
#endif
	union psmi_envvar_val timeout_intval;
	psm2_ep_t tmp;
	psm2_mq_t mmq;

	PSM2_LOG_MSG("entering");
	PSMI_ERR_UNLESS_INITIALIZED(ep);
	psmi_assert_always(ep->mctxt_master == ep);

	PSMI_LOCK(psm3_creation_lock);

	psm3_am_fini_internal(ep);

	if (psm3_opened_endpoint == NULL) {
		err = psm3_handle_error(NULL, PSM2_EP_WAS_CLOSED,
					"PSM Endpoint is closed or does not exist");
		PSM2_LOG_MSG("leaving");
		PSMI_UNLOCK(psm3_creation_lock);
		return err;
	}

	tmp = psm3_opened_endpoint;
	while (tmp && tmp != ep) {
		tmp = tmp->user_ep_next;
	}
	if (!tmp) {
		err = psm3_handle_error(NULL, PSM2_EP_WAS_CLOSED,
					"PSM Endpoint is closed or does not exist");
		PSM2_LOG_MSG("leaving");
		PSMI_UNLOCK(psm3_creation_lock);
		return err;
	}

	if (! psm3_getenv("PSM3_CLOSE_TIMEOUT",
		    "End-point close timeout over-ride.",
		    PSMI_ENVVAR_LEVEL_HIDDEN, PSMI_ENVVAR_TYPE_UINT,
		    (union psmi_envvar_val)0, &timeout_intval)) {
		timeout_in = timeout_intval.e_uint * NSEC_PER_SEC;
	} else if (timeout_in > 0) {
		/* The timeout parameter provides the minimum timeout. A heuristic
		 * is used to scale up the timeout linearly with the number of
		 * endpoints, and we allow one second per 100 endpoints. */
		timeout_in = max(timeout_in, (ep->connections * NSEC_PER_SEC) / 100);
	}

	if (timeout_in > 0 && timeout_in < PSMI_MIN_EP_CLOSE_TIMEOUT)
		timeout_in = PSMI_MIN_EP_CLOSE_TIMEOUT;

	/* Infinite and excessive close time-out are limited here to a max.
	 * The "rationale" is that there is no point waiting around forever for
	 * graceful termination. Normal (or forced) process termination should clean
	 * up the context state correctly even if termination is not graceful. */
	if (timeout_in <= 0 || timeout_in > PSMI_MAX_EP_CLOSE_TIMEOUT)
		timeout_in = PSMI_MAX_EP_CLOSE_TIMEOUT;
	_HFI_PRDBG("Closing endpoint %p with force=%s and to=%.2f seconds and "
		   "%d connections\n",
		   ep, mode == PSM2_EP_CLOSE_FORCE ? "YES" : "NO",
		   (double)timeout_in / 1e9, (int)ep->connections);

	/* XXX We currently cheat in the sense that we leave each PTL the allowed
	 * timeout.  There's no good way to do this until we change the PTL
	 * interface to allow asynchronous finalization
	 */


	/* Check if transfer ownership of receive thread is needed before closing ep.
	 * In case of PSM3_MULTI_EP support receive thread is created and assigned
	 * to first opened endpoint. Receive thread is killed when closing this
	 * endpoint.
	 */
	if (ep->user_ep_next != NULL) {
		/* Receive thread will be transfered and assigned to ep->user_ep_next
		 * only if currently working receive thread (which will be killed) is
		 * assigned to ep and there isn't any assigned to ep->user_ep_next.
		 */
		if ((psm3_ptl_ips_rcvthread.is_enabled(ep->ptl_ips.ptl)) &&
		    (!psm3_ptl_ips_rcvthread.is_enabled(ep->user_ep_next->ptl_ips.ptl)))
			psm3_ptl_ips_rcvthread.transfer_ownership(ep->ptl_ips.ptl, ep->user_ep_next->ptl_ips.ptl);
	}

	/*
	 * Before freeing the master ep itself,
	 * remove it from the global linklist.
	 * We do it here to let atexit handler in ptl_am directory
	 * to search the global linklist and free the shared memory file.
	 */
	if (psm3_opened_endpoint == ep) {
		/* Removing ep from global endpoint list. */
		psm3_opened_endpoint = ep->user_ep_next;
	} else {
		tmp = psm3_opened_endpoint;
		while (tmp->user_ep_next != ep) {
			tmp = tmp->user_ep_next;
		}
		/* Removing ep from global endpoint list. */
		tmp->user_ep_next = ep->user_ep_next;
	}
	psm3_opened_endpoint_count--;

	/*
	 * This do/while loop is used to close and free memory of endpoints.
	 *
	 * If MULTIRAIL feature is disable this loop will be passed only once
	 * and only endpoint passed in psm3_ep_close will be closed/removed.
	 *
	 * If MULTIRAIL feature is enabled then this loop will be passed
	 * multiple times (depending on number of rails). The order in which
	 * endpoints will be closed is shown below:
	 *
	 *                      |--this is master endpoint in case of multirail
	 *	                |  this endpoint is passed to psm3_ep_close and
	 *			V  this is only endpoint known to user.
	 *   +<-Ep0<-Ep1<-Ep2<-Ep3
	 *   |__________________|	Ep3->mctxt_prev points to Ep2
	 *	(3)  (2)  (1)  (4)	Ep2->mctxt_prev points to Ep1
	 *	 ^			Ep1->mctxt_prev points to Ep0
	 *	 |			Ep0->mctxt_prev points to Ep3 (master ep)
	 *	 |
	 *       |---- order in which endpoints will be closed.
	 *
	 * Closing MULTIRAILs starts by closing secondaries (Ep2, Ep1, Ep0)
	 * If MULTIRAIL is enabled then Ep3->mctxt_prev will point to Ep2, if
	 * feature is disabled then Ep3->mctxt_prev will point to Ep3 and
	 * do/while loop will have one pass.
	 *
	 * In case of MULTIRAIL enabled Ep3 which is master endpoint will be
	 * closed as the last one.
	 */
	mmq = ep->mq;
	if (mmq) {
		// in case mq_finalize not called, need to get stats out
		// it will be a noop if called twice
		psm3_mq_finalize(mmq);
	}
	tmp = ep->mctxt_prev;
	do {
		ep = tmp;
		tmp = ep->mctxt_prev;

		PSMI_LOCK(ep->mq->progress_lock);

		if (psm3_ep_device_is_enabled(ep, PTL_DEVID_AMSH))
			err =
			    psm3_ptl_amsh.fini(ep->ptl_amsh.ptl, mode,
					       timeout_in);

		if ((err == PSM2_OK || err == PSM2_TIMEOUT) &&
		    psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS))
			err =
			    psm3_ptl_ips.fini(ep->ptl_ips.ptl, mode,
					      timeout_in);
		PSM_MCTXT_REMOVE(ep);
		/* If there's timeouts in the disconnect requests,
		 * still make sure that we still get to close the
		 *endpoint and mark it closed */
		psm3_ep_close_device(ep);

		psm3_epid_remove_all(ep);
		psmi_free(ep->epaddr);
		psmi_free(ep->context_mylabel);

		PSMI_UNLOCK(ep->mq->progress_lock);

		ep->mq = NULL;

		psmi_free(ep);

	} while ((err == PSM2_OK || err == PSM2_TIMEOUT) && tmp != ep);

	if (mmq) {
		psmi_destroy_lock(&(mmq->progress_lock));
		err = psm3_mq_free(mmq);
	}

	PSMI_UNLOCK(psm3_creation_lock);

	if (_HFI_PRDBG_ON) {
		_HFI_PRDBG_ALWAYS("Closed endpoint in %.3f secs\n",
				 (double)cycles_to_nanosecs(get_cycles() -
				 t_start) / NSEC_PER_SEC);
	}

	if (psm3_opened_endpoint_count == 0) {
		PSM3_GPU_EP_CLOSE();
		devnids_destroy();
	}

	PSM2_LOG_MSG("leaving");
	return err;
}

static
psm2_error_t
psm3_ep_open_device(const psm2_ep_t ep, const struct psm3_ep_open_opts *opts,
		    const psm2_uuid_t unique_job_key)
{
	psm2_error_t err = PSM2_OK;

	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS)) {
		ep->out_sl = opts->outsl;
		if ((err =
		     psm3_context_open(ep, opts->unit, opts->port, opts->addr_index,
				       unique_job_key,
				       (uint16_t) opts->network_pkey,
				       opts->timeout)) != PSM2_OK)
			goto fail;
		psmi_assert(ep->addr.fmt == psm3_epid_addr_fmt(ep->epid));
		// ep->addr_fmt set by psm3_context_open
		psmi_assert(ep->addr_fmt == psm3_epid_addr_fmt(ep->epid));
	} else if (psm3_ep_device_is_enabled(ep, PTL_DEVID_AMSH)) {
		/* shm-only, meaning one 1 node max */
		ep->epid = psm3_epid_pack_shm(unique_job_key);
		ep->addr_fmt = psm3_epid_addr_fmt(ep->epid);
		ep->dev_name = "shm";
		_HFI_VDBG("construct epid shm-only: %s\n",
					psm3_epid_fmt_internal(ep->epid, 0));
	} else {
		/* Self-only, meaning only 1 proc max */
		ep->epid = psm3_epid_pack_self();
		ep->addr_fmt = psm3_epid_addr_fmt(ep->epid);
		ep->dev_name = "self";
		_HFI_VDBG("construct epid self-only: %s\n",
					psm3_epid_fmt_internal(ep->epid, 0));
	}

fail:
	return err;
}

static
void
psm3_ep_close_device(const psm2_ep_t ep)
{
	if (psm3_ep_device_is_enabled(ep, PTL_DEVID_IPS))
		psm3_context_close(ep);
}

/* Get a list of PTLs we want to use.  The order is important, it affects
 * whether node-local processes use shm or ips
 * We fetch and parse PSM3_DEVICES each time this is called so that
 * a middleware could potentially open EPs with only a subset of
 * devices, such as a shm-only EP if the middleware will only use the
 * given EP for intra-node comms
 */
psm2_error_t
psm3_parse_devices(int devices[PTL_MAX_INIT])
{
	char *devstr = NULL;
	char *b_new, *e, *ee, *b;
	psm2_error_t err = PSM2_OK;
	int len;
	int i = 0;
	union psmi_envvar_val devs;
	static int have_value = 0;
	static int saved[PTL_MAX_INIT];

	// only parse once so doesn't appear in PSM3_VERBOSE_ENV multiple times
	if (have_value) {
		for (i=0; i < PTL_MAX_INIT; i++)
			devices[i] = saved[i];
		return PSM2_OK;
	}

	/* See which ptl devices we want to use for this ep to be opened */
	psm3_getenv("PSM3_DEVICES",
		    "List of PSM-level device protocols (self, shm, and/or nic)",
		    PSMI_ENVVAR_LEVEL_USER,
		    PSMI_ENVVAR_TYPE_STR,
		    (union psmi_envvar_val)PSMI_DEVICES_DEFAULT, &devs);

	psmi_assert_always(devs.e_str != NULL);
	len = strlen(devs.e_str) + 1;

	for (i = 0; i < PTL_MAX_INIT; i++)
		devices[i] = -1;

	devstr = (char *)psmi_calloc(PSMI_EP_NONE, UNDEFINED, 2, len);
	if (devstr == NULL)
		goto fail;

	b_new = (char *)devstr;
	e = b_new + len;
	strncpy(e, devs.e_str, len);
	ee = e + len;
	i = 0;
	while (e < ee && *e && i < PTL_MAX_INIT) {
		while (*e && !isalpha(*e))
			e++;
		b = e;
		while (*e && isalpha(*e))
			e++;
		*e = '\0';
		if (*b) {
			if (!strcasecmp(b, "self")) {
				devices[i++] = PTL_DEVID_SELF;
				b_new = strcpy(b_new, "self,");
				b_new += 5;
			} else if (!strcasecmp(b, "shm") ||
					!strcasecmp(b, "shmem") ||
					!strcasecmp(b, "amsh")) {
				devices[i++] = PTL_DEVID_AMSH;
				strcpy(b_new, "amsh,");
				b_new += 5;
			} else if (!strcasecmp(b, "hfi") ||
					!strcasecmp(b, "nic") ||
					!strcasecmp(b, "ipath") ||
					!strcasecmp(b, "ips")) {
				devices[i++] = PTL_DEVID_IPS;
				strcpy(b_new, "ips,");
				b_new += 4;
			} else {
				err = psm3_handle_error(NULL, PSM2_PARAM_ERR,
							"'%s' set in environment variable PSM_PTL_DEVICES=\"%s\" "
							"is not one of the recognized PTL devices (%s)",
							b, devs.e_str,
							PSMI_DEVICES_DEFAULT);
				goto fail;
			}
			e++;
		}
	}
	if (b_new != devstr)	/* we parsed something, remove trailing comma */
		*(b_new - 1) = '\0';

	_HFI_PRDBG("PSM Device allocation order: %s\n", devstr);
	for (i=0; i < PTL_MAX_INIT; i++)
		saved[i] = devices[i];
	have_value = 1;
fail:
	if (devstr != NULL)
		psmi_free(devstr);
	return err;

}

int psm3_device_is_enabled(const int devid_enabled[PTL_MAX_INIT], int devid)
{
	int i;
	for (i = 0; i < PTL_MAX_INIT; i++)
		if (devid_enabled[i] == devid)
			return 1;
	return 0;
}

int psm3_ep_device_is_enabled(const psm2_ep_t ep, int devid)
{
	return psm3_device_is_enabled(ep->devid_enabled, devid);
}

#ifdef PSM_HAVE_RNDV_MOD
#ifdef PSM_HAVE_GPU
// used for GdrCopy

// given an ep this returns the "next one".
// It loops through all the multi-rail/multi-QP EPs in a given user opened EP
// 1st, then it goes to the next user opened EP (multi-EP) and loops through
// it's multi-rail/mult-QP EPs.
// When it hits the last rail of the last user opened EP, it goes back to
// the 1st rail of the 1st user opened EP.
// caller must hold creation_lock
static psm2_ep_t next_ep(psm2_ep_t ep)
{
       //mctxt_next is the circular list of rails/QPs in a given user EP
       //mctxt_master is the 1st in the list, when we get back to the 1st
       //go to the next user EP
       ep = ep->mctxt_next;
       if (ep->mctxt_master != ep)
               return ep;
       //user_ep_next is a linked list of user opened EPs.  End of list is NULL
       //when hit end of list, go back to 1st (psm3_opened_endpoint)
       //for each user opened EP, the entry on this list is the 1st rail within
       //the EP
       ep = ep->user_ep_next;
       if (ep)
               return ep;
       else
               return psm3_opened_endpoint;
}

// determine if ep is still valid (can't dereference or trust ep given)
// caller must hold creation_lock
static int valid_ep(psm2_ep_t ep)
{
	psm2_ep_t e1 = psm3_opened_endpoint;

	while (e1) {	// user opened ep's - linear list ending in NULL
		psm2_ep_t e2 = e1;
		//check mtcxt list (multi-rail within user opened ep)
		do {
			if (e2 == ep)
				return 1;
			e2 = e2->mctxt_next;
		} while (e2 != e1);	// circular list
		e1 = e1->user_ep_next;
	}
	return 0;	// not found
}

// advance ep to the next.  However it's possible ep is stale and
// now closed/freed, so make sure it's good.  good_ep is at least one
// known good_ep and lets us avoid search some of the time (or if only 1 EP)
// caller must hold creation_lock
static psm2_ep_t next_valid_ep(psm2_ep_t ep, psm2_ep_t good_ep)
{
	if (ep == good_ep || valid_ep(ep))
		return next_ep(ep);
	else
		return good_ep;
}

// TBD - seems this function, and perhaps enabling functions above
// would better fit in psm_mr_cache.c  PSM_VERBS and PSM_SOCKETS uses it
/*
 * Evict some space in given cache (only GPU needs this)
 * If nvidia_p2p_get_pages reports out of BAR space (perhaps prematurely),
 * we need to evict from other EPs too.
 * So we rotate among all eps (rails or QPs) in our user opened EP for eviction.
 * length - amount attempted in pin/register which just failed
 * access - indicates if IS_GPU_ADDR or not (rest ignored)
 * returns:
 * 	>0 bytes evicted if some evicted
 * 	-1 if nothing evicted (errno == ENOENT means nothing evictable found)
 * 	ENOENT also used when access is not for GPU
 * The caller will have the progress_lock, we need the creation_lock
 * to walk the list of EPs outside our own MQ.  However creation_lock
 * is above the progress_lock in lock heirarchy, so we use a LOCK_TRY
 * to avoid deadlock in the rare case where another thread
 * has creation_lock and is trying to get progress_lock (such as during
 * open_ep, close_ep or rcvthread).
 */
int64_t psm3_gpu_evict_some(psm2_ep_t ep, uint64_t length, int access)
{
	static __thread psm2_ep_t last_evict_ep;	// among all eps
	static __thread psm2_ep_t last_evict_myuser_ep;	// in my user ep
	int64_t evicted = 0;
	int ret;

	if (! (access & IBV_ACCESS_IS_GPU_ADDR)) {
		errno = ENOENT;
		return -1;	// only need evictions on GPU addresses
	}
	if (! last_evict_ep) {	// first call only
		last_evict_ep = ep;
		last_evict_myuser_ep = ep;
	}
	// 1st try to evict from 1st rail/QP in our opened EP (gdrcopy and MRs)
	ret = psm3_rv_evict_gpu_amount(ep->mctxt_master->rv,
					 max(psm3_gpu_cache_evict, length), 0);
	if (ret > 0)
		evicted = ret;

	// next rotate among other rails/QPs in our opened ep (MRs)
	last_evict_myuser_ep = last_evict_myuser_ep->mctxt_next;
	if (last_evict_myuser_ep != ep->mctxt_master) {
		ret = psm3_rv_evict_gpu_amount(last_evict_myuser_ep->rv,
						 max(psm3_gpu_cache_evict, length),
						 0);
		if (ret > 0)
			evicted += ret;
	}
	if (evicted >= length)
		return evicted;

	// now try other opened EPs
	if (PSMI_LOCK_TRY(psm3_creation_lock))
		goto done;
	// last_evict_ep could point to an ep which has since been closed/freed
	last_evict_ep = next_valid_ep(last_evict_ep, ep);
	if (last_evict_ep->mctxt_master != ep->mctxt_master) {
		if (!PSMI_LOCK_TRY(last_evict_ep->mq->progress_lock)) {
			ret = psm3_rv_evict_gpu_amount(last_evict_ep->rv,
					max(psm3_gpu_cache_evict, length), 0);
			PSMI_UNLOCK(last_evict_ep->mq->progress_lock);
			if (ret > 0)
				evicted += ret;
		}
	} else {
		ret = psm3_rv_evict_gpu_amount(last_evict_ep->rv,
						 max(psm3_gpu_cache_evict, length),
						 0);
		if (ret > 0 )
			evicted += ret;
	}
	PSMI_UNLOCK(psm3_creation_lock);
done:
	if (! evicted) {
		errno = ENOENT;
		return -1;
	}
	return evicted;
}
#endif /* PSM_HAVE_GPU */
#endif /* PSM_HAVE_RNDV_MOD */
