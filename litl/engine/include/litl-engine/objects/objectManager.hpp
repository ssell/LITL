#ifndef LITL_ENGINE_OBJECT_MANAGER_H__
#define LITL_ENGINE_OBJECT_MANAGER_H__

#include <memory>

namespace litl
{
    class ObjectManager
    {
    public:



    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif