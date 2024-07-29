/*
 * Copyright (C) 2022-2024 by Cornelis Networks.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rdma/opx/fi_opx_cq_ops_table.h"

/* HDRQ_MASK = 2k value (2047 * 0x20) */
/* CAPS =  FI_OPX_COMMS_NONE (runtime) */
/* ----- OFI_RELIABILITY_KIND_ONLOAD */
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_UNSPEC,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_CONTEXT, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_MSG,     OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_DATA,    OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_TAGGED,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE)

/* CAPS = FI_OPX_COMMS_LOCAL (only local) */
/* ----- OFI_RELIABILITY_KIND_ONLOAD */
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_UNSPEC,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_CONTEXT, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_MSG,     OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_DATA,    OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_TAGGED,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL)

/* CAPS = FI_OPX_COMMS_REMOTE (only remote) */
/* ----- OFI_RELIABILITY_KIND_ONLOAD */
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_UNSPEC,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_CONTEXT, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_MSG,     OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_DATA,    OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_TAGGED,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE)

/* CAPS = FI_OPX_COMMS_LOCAL_REMOTE (local and remote) */
/* ----- OFI_RELIABILITY_KIND_ONLOAD */
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_UNSPEC,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_CONTEXT, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_MSG,     OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_DATA,    OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE)
	FI_OPX_CQ_SPECIALIZED_FUNC_NON_LOCKING(FI_CQ_FORMAT_TAGGED,  OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE)


static struct fi_ops_cq fi_opx_cq_non_locking_2048_ops_table[] = {

	// Format: FI_CQ_FORMAT_UNSPEC
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_UNSPEC, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_UNSPEC, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_UNSPEC, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_UNSPEC, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE),

	// Format: FI_CQ_FORMAT_CONTEXT
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_CONTEXT, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_CONTEXT, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_CONTEXT, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_CONTEXT, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE),

	// Format: FI_CQ_FORMAT_MSG
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_MSG, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_MSG, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_MSG, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_MSG, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE),

	//Format: FI_CQ_FORMAT_DATA
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_DATA, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_DATA, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_DATA, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_DATA, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE),

	// Format: FI_CQ_FORMAT_TAGGED
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_TAGGED, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_NONE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_TAGGED, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_TAGGED, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_REMOTE),
	FI_OPX_CQ_OPS_STRUCT_INIT(FI_CQ_FORMAT_TAGGED, FI_OPX_LOCK_NOT_REQUIRED, OFI_RELIABILITY_KIND_ONLOAD, FI_OPX_HDRQ_MASK_2048, FI_OPX_COMMS_LOCAL_REMOTE),
};


static op_matrix_t *fi_opx_cq_non_locking_2048_ops = (op_matrix_t *)&fi_opx_cq_non_locking_2048_ops_table;

struct fi_ops_cq * fi_opx_cq_select_non_locking_2048_ops(const enum fi_cq_format format,
						const enum ofi_reliability_kind reliability,
						const uint64_t comm_caps)
{
	return &(*fi_opx_cq_non_locking_2048_ops)[format][0][comm_caps];
}
