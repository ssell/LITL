#ifndef LITL_CORE_AUTHORITY_H__
#define LITL_CORE_AUTHORITY_H__

namespace litl
{
    /// <summary>
    /// Used to protect public methods.
    /// </summary>
    template<typename T>
    class Authority
    {
    public:

        Authority(Authority const&) = default;

    private:

        friend T;
        Authority() = default;
    };
}

#endif