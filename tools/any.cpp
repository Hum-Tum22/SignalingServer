#include "any.h"

using namespace ownAny;
//=============================================================================
// class bad_Any_cast
//=============================================================================

const char* bad_Any_cast::what() const noexcept
{
    return "bad Any_cast";
}

//=============================================================================
// class Any
//=============================================================================

constexpr Any::Any() noexcept
    : m_manager(nullptr)
{ }

Any::Any(const Any& other)
{
    if (!other.has_value()) {
        m_manager = nullptr;
    }
    else {
        ManageArg marg;
        marg._Any = this;
        other.m_manager(MOClone, &other, &marg);
    }
}

Any::Any(Any&& other) noexcept
{
    if (!other.has_value()) {
        m_manager = nullptr;
    }
    else {
        ManageArg marg;
        marg._Any = this;
        other.m_manager(MOXfer, &other, &marg);
    }
}

Any::~Any()
{
    reset();
}

Any& Any::operator=(const Any& rhs)
{
    *this = Any(rhs);
    return *this;
}

Any& Any::operator=(Any&& rhs) noexcept
{
    if (!rhs.has_value()) {
        reset();
    }
    else if (this != &rhs) {
        reset();
        ManageArg marg;
        marg._Any = this;
        rhs.m_manager(MOXfer, &rhs, &marg);
    }
    return *this;
}

void Any::reset() noexcept
{
    if (has_value()) {
        m_manager(MODestroy, this, nullptr);
        m_manager = nullptr;
    }
}

void Any::swap(Any& rhs) noexcept
{
    if (!has_value() && !rhs.has_value()) {
        return;
    }

    if (has_value() && rhs.has_value()) {
        if (this == &rhs) {
            return;
        }

        Any tmp;
        ManageArg marg;
        marg._Any = &tmp;
        rhs.m_manager(MOXfer, &rhs, &marg);
        marg._Any = &rhs;
        m_manager(MOXfer, this, &marg);
        marg._Any = this;
        tmp.m_manager(MOXfer, &tmp, &marg);
    }
    else {
        Any* empty = !has_value() ? this : &rhs;
        Any* full = !has_value() ? &rhs : this;
        ManageArg marg;
        marg._Any = empty;
        full->m_manager(MOXfer, full, &marg);
    }
}

bool Any::has_value() const noexcept
{
    return m_manager != nullptr;
}

#if __cpp_rtti
const std::type_info& Any::type() const noexcept
{
    if (!has_value()) {
        return typeid(void);
    }
    ManageArg marg;
    m_manager(MOGetTypeInfo, this, &marg);
    return *marg._typeinfo;
}
#endif // __cpp_rtti