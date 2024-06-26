/* SPDX-License-Identifier: BSD-2-Clause */
/*******************************************************************************
 * Copyright 2017-2018, Fraunhofer SIT sponsored by Infineon Technologies AG
 * All rights reserved.
 *******************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h" // IWYU pragma: keep
#endif

#include <stdlib.h>           // for free, EXIT_FAILURE, EXIT_SUCCESS, size_t
#include <string.h>           // for memcmp

#include "tss2_common.h"      // for BYTE, TSS2_RC
#include "tss2_esys.h"        // for Esys_TR_GetName, ESYS_CONTEXT, ESYS_TR_...
#include "tss2_mu.h"          // for Tss2_MU_TPM2_HANDLE_Marshal
#include "tss2_tpm2_types.h"  // for TPM2B_NAME, TPM2_RH_OWNER

#define LOGMODULE test
#include "util/log.h"         // for goto_if_error, LOG_ERROR

/** This tests the Esys_TR_FromTPMPublic and Esys_TR_GetName functions by
 *  creating an NV Index and then attempting to retrieve an ESYS_TR object for
 *  it.
 *  Then we call Esys_TR_GetName to see if the correct public name has been
 * retrieved.
 *
 * Tested ESYS commands:
 *
 * @param[in,out] ectx The ESYS_CONTEXT.
 * @retval EXIT_FAILURE
 * @retval EXIT_SUCCESS
 */

int
test_esys_tr_getName_hierarchy(ESYS_CONTEXT * ectx)
{
    TSS2_RC r;

    TPM2B_NAME name1, *name2;
    size_t offset = 0;

    r = Tss2_MU_TPM2_HANDLE_Marshal(TPM2_RH_OWNER, &name1.name[0],
                                    sizeof(name1.name), &offset);
    goto_if_error(r, "Marshaling name", error);
    name1.size = offset;

    r = Esys_TR_GetName(ectx, ESYS_TR_RH_OWNER, &name2);
    goto_if_error(r, "TR get name", error);

    if (name1.size != name2->size ||
        memcmp(&name1.name[0], &name2->name[0], name1.size) != 0)
    {
        free(name2);
        LOG_ERROR("Names mismatch between NV_GetPublic and TR_GetName");
        return EXIT_FAILURE;
    }

    free(name2);

    return EXIT_SUCCESS;

 error:
    return EXIT_FAILURE;
}

int
test_invoke_esys(ESYS_CONTEXT * esys_context) {
    return test_esys_tr_getName_hierarchy(esys_context);
}
