/* SPDX-License-Identifier: BSD-2-Clause */
/***********************************************************************;
 * Copyright (c) 2015 - 2017, Intel Corporation
 * All rights reserved.
 ***********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h" // IWYU pragma: keep
#endif

#include "sysapi_util.h"      // for _TSS2_SYS_CONTEXT_BLOB, syscontext_cast
#include "tss2_common.h"      // for TSS2_RC, TSS2_SYS_RC_BAD_REFERENCE, TSS...
#include "tss2_mu.h"          // for Tss2_MU_TPML_CC_Marshal, Tss2_MU_UINT16...
#include "tss2_sys.h"         // for TSS2_SYS_CONTEXT, TSS2L_SYS_AUTH_COMMAND
#include "tss2_tpm2_types.h"  // for TPML_CC, TPMI_ALG_HASH, TPMI_RH_PROVISION

TSS2_RC Tss2_Sys_SetCommandCodeAuditStatus_Prepare(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_RH_PROVISION auth,
    TPMI_ALG_HASH auditAlg,
    const TPML_CC *setList,
    const TPML_CC *clearList)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!ctx || !setList  || !clearList)
        return TSS2_SYS_RC_BAD_REFERENCE;

    if (IsAlgorithmWeak(auditAlg, 0))
        return TSS2_SYS_RC_BAD_VALUE;

    rval = CommonPreparePrologue(ctx, TPM2_CC_SetCommandCodeAuditStatus);
    if (rval)
        return rval;

    rval = Tss2_MU_UINT32_Marshal(auth, ctx->cmdBuffer,
                                  ctx->maxCmdSize,
                                  &ctx->nextData);
    if (rval)
        return rval;

    rval = Tss2_MU_UINT16_Marshal(auditAlg, ctx->cmdBuffer,
                                  ctx->maxCmdSize,
                                  &ctx->nextData);
    if (rval)
        return rval;

    rval = Tss2_MU_TPML_CC_Marshal(setList, ctx->cmdBuffer,
                                   ctx->maxCmdSize,
                                   &ctx->nextData);
    if (rval)
        return rval;

    rval = Tss2_MU_TPML_CC_Marshal(clearList, ctx->cmdBuffer,
                                   ctx->maxCmdSize,
                                   &ctx->nextData);
    if (rval)
        return rval;

    ctx->decryptAllowed = 0;
    ctx->encryptAllowed = 0;
    ctx->authAllowed = 1;

    return CommonPrepareEpilogue(ctx);
}

TSS2_RC Tss2_Sys_SetCommandCodeAuditStatus_Complete (
    TSS2_SYS_CONTEXT *sysContext)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);

    if (!ctx)
        return TSS2_SYS_RC_BAD_REFERENCE;

    return CommonComplete(ctx);
}

TSS2_RC Tss2_Sys_SetCommandCodeAuditStatus(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_RH_PROVISION auth,
    TSS2L_SYS_AUTH_COMMAND const *cmdAuthsArray,
    TPMI_ALG_HASH auditAlg,
    const TPML_CC *setList,
    const TPML_CC *clearList,
    TSS2L_SYS_AUTH_RESPONSE *rspAuthsArray)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!setList || !clearList)
        return TSS2_SYS_RC_BAD_REFERENCE;

    rval = Tss2_Sys_SetCommandCodeAuditStatus_Prepare(sysContext, auth, auditAlg,
                                                      setList, clearList);
    if (rval)
        return rval;

    rval = CommonOneCall(ctx, cmdAuthsArray, rspAuthsArray);
    if (rval)
        return rval;

    return Tss2_Sys_SetCommandCodeAuditStatus_Complete(sysContext);
}
