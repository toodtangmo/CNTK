//
// <copyright company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//

#pragma once

#include "MemoryProvider.h"

namespace Microsoft { namespace MSR { namespace CNTK {

    // Currently a hack because we do not know the device id in advance.
    class SubstitutingMemoryProvider : public MemoryProvider
    {
        MemoryProviderPtr m_provider;

    public:
        SubstitutingMemoryProvider() {}

        void SetMemoryProvider(MemoryProviderPtr p)
        {
            m_provider = p;
        }

        virtual void* Alloc(size_t elementSize, size_t numberOfElements) override
        {
            return m_provider->Alloc(elementSize, numberOfElements);
        }

        virtual void Free(void* p) override
        {
            return m_provider->Free(p);
        }
    };

    typedef std::shared_ptr<SubstitutingMemoryProvider> SubstitutingMemoryProviderPtr;
}}}