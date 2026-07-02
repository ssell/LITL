#ifndef LITL_CORE_AUTHORITY_H__
#define LITL_CORE_AUTHORITY_H__

namespace litl
{
    /// <summary>
    /// Used to protect public methods.
    /// 
    /// In many cases this can be replaced by using a friend grant.
    /// However authority provides two improvements over standard friendship:
    /// 
    /// 1) Access to a method can be granted on behalf of the authority.
    /// 2) It more clearly shows intent. If a class has multiple friends, then any of those friends can access the protected/private method. But by
    /// using authority, only the explicitly allowed class (or a caller on its behalf) may use the method.
    /// </summary>
    template<typename T>
    class Authority
    {
    public:

        /// <summary>
        /// Allow the authority to passed on to another caller.
        /// </summary>
        /// <param name=""></param>
        Authority(Authority const&) = default;

        /// <summary>
        /// Mints an authority badge for the specified type.
        /// Only available for testing purposes.
        /// </summary>
        /// <returns></returns>
        static Authority<T> mint()
        {
#ifdef LITL_TEST_SUITE
            return Authority<T>{};
#else
            throw;
#endif
        }

    private:

        friend T;
        Authority() = default;
    };
}

#endif