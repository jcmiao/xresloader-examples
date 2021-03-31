#ifndef _POOL_H_
#define _POOL_H_

#include <google/protobuf/arena.h>
#include <map>
#include <list>

google::protobuf::Arena* InitArena(char* initial_block, size_t initial_block_size);
google::protobuf::Arena* GetArena();

char* InitConfigManagerPtr(size_t objsize);
char* GetConfigManagerPtr();

template <class _Ty>
class MyAllocator {
public:
    using value_type = _Ty;

    typedef _Ty* pointer;
    typedef const _Ty* const_pointer;

    typedef _Ty& reference;
    typedef const _Ty& const_reference;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal                        = std::true_type;

    _Ty* address(_Ty& _Val) const noexcept {
        return addressof(_Val);
    }

    const _Ty* address(const _Ty& _Val) const noexcept {
        return addressof(_Val);
    }

    constexpr MyAllocator() noexcept {}

    constexpr MyAllocator(const MyAllocator&) 
    {

    }
    template <class _Other>
    constexpr MyAllocator(const MyAllocator<_Other>&) noexcept {}

    void deallocate(_Ty* const _Ptr, const size_t _Count) {
        // no overflow check on the following multiply; we assume _Allocate did that check
        // _Deallocate<_New_alignof<_Ty>>(_Ptr, sizeof(_Ty) * _Count);
        // mPoolMap[_Count].push_back(const_cast<char*>(_Ptr));
        mPoolMap[_Count].push_back((char*)(_Ptr));
    }

    _Ty* allocate(const size_t _Count) {
        auto& m = mPoolMap[_Count];
        if (m.empty())
        {
            return (_Ty*)(new char[sizeof(_Ty) * _Count]);
        }
        auto p = m.front();
        m.pop_front();
        return (_Ty*)(p);
        // return static_cast<_Ty*>(_Allocate<_New_alignof<_Ty>>(_Get_size_of_n<sizeof(_Ty)>(_Count)));
    }

    _Ty* allocate(
        const size_t _Count, const void*) {
        return allocate(_Count);
    }

    template <class _Objty, class... _Types>
    void construct(_Objty* const _Ptr, _Types&&... _Args) {
        ::new (const_cast<void*>(static_cast<const volatile void*>(_Ptr))) _Objty(std::forward<_Types>(_Args)...);
    }

    template <class _Uty>
    void destroy(_Uty* const _Ptr) {
        _Ptr->~_Uty();
    }

    size_t max_size() const noexcept {
        return static_cast<size_t>(-1) / sizeof(_Ty);
    }

private:
    std::map<int, std::list<char*>> mPoolMap;
};

#endif /* _POOL_H_ */