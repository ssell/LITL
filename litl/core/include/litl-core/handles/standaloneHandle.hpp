#ifndef LITL_CORE_STANDALONE_HANDLE_H__
#define LITL_CORE_STANDALONE_HANDLE_H__

/// <summary>
/// A basic standalone handle. Used when a singular opaque handle is needed without a related pool.
/// </summary>
using LITLHandle = void*;

namespace litl
{
#define DEFINE_LITL_HANDLE(Name) struct Name { LITLHandle handle; }
#define LITL_PACK_HANDLE(HandleType, HandleObj) HandleType { HandleObj }
#define LITL_UNPACK_HANDLE(HandleType, HandleObj) static_cast<HandleType*>(HandleObj.handle)
#define LITL_UNPACK_HANDLE_PTR(HandleType, HandlePtr) static_cast<HandleType*>(HandlePtr->handle)
}


#endif