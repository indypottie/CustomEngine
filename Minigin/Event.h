#pragma once

#include <any>
#include <cstdint>
#include <string>
#include <variant>
#include <type_traits>
#include <utility>

// Forward declare game types here only as pointers (if needed):
// struct TileCoords; // don't include full definition here

#include "CollisionInfo.h"

// EventArg stores known engine types + a fallback for arbitrary game types via std::any
struct EventArg
{
    using Value = std::variant<
        std::monostate,
        int,
        float,
        bool,
        const char*,
        std::string,
        CollisionInfo,
        std::any // for arbitrary types
    >;

    Value value;

    EventArg() = default;

    // Trait to detect supported engine types
    template<typename T>
    struct is_supported_type : std::disjunction<
        std::is_same<std::decay_t<T>, int>,
        std::is_same<std::decay_t<T>, float>,
        std::is_same<std::decay_t<T>, bool>,
        std::is_same<std::decay_t<T>, const char*>,
        std::is_same<std::decay_t<T>, std::string>,
        std::is_same<std::decay_t<T>, CollisionInfo>
    > {
    };

    // Constructor for supported types (store directly in variant)
    template <typename T>
    EventArg(T&& _value)
        requires is_supported_type<T>::value
    : value(std::forward<T>(_value))
    {
    }

    // Constructor for unsupported types (store in std::any)
    template <typename T>
    EventArg(T&& _value)
        requires (!is_supported_type<T>::value)
    : value(std::any(std::forward<T>(_value)))
    {
    }

    // WARNING: Use "TryGetEventArg" for safety 
    template <typename T>
    T Get() const
    {
        if constexpr (is_supported_type<T>::value)
        {
            return std::get<T>(value);
        }
        else
        {
            if (std::holds_alternative<std::any>(value))
            {
                return std::any_cast<T>(std::get<std::any>(value));
            }

            throw std::bad_any_cast();
        }
    }

    // Safe try-get (returns false if cast fails)
    template<typename T>
    static bool TryGetEventArg(const EventArg& arg, T& outValue)
    {
        try
        {
            outValue = arg.Get<T>();
            return true;
        }
        catch (const std::bad_variant_access&) { return false; }
        catch (const std::bad_any_cast&) { return false; }
    }
};

using EventId = unsigned int;

struct Event
{
    const EventId id;

    static constexpr uint8_t MAX_ARGS = 8;
    uint8_t nbArgs = 0;
    EventArg args[MAX_ARGS];

    explicit Event(EventId _id) : id(_id) {}

    template <typename... Args>
    Event(EventId _id, Args&&... _args) : id(_id), nbArgs(0)
    {
        SetArgs(std::forward<Args>(_args)...);
    }

private:

    template <typename T, typename... Rest>
    void SetArgs(T&& first, Rest&&... rest)
    {
        if (nbArgs < MAX_ARGS)
        {
            args[nbArgs++] = EventArg(std::forward<T>(first));
            SetArgs(std::forward<Rest>(rest)...);
        }
    }

    void SetArgs() {} // Terminate recursion
};

// Compile-time sdbm hash
template <int length> struct sdbm_hash
{
	consteval static unsigned int _calculate(const char* const text, unsigned int& value)
	{
		const unsigned int character = sdbm_hash<length - 1>::_calculate(text, value);
		value = character + (value << 6) + (value << 16) - value;
		return text[length - 1];
	}

	consteval static unsigned int calculate(const char* const text)
	{
		unsigned int value = 0;
		const auto character = _calculate(text, value);
		return character + (value << 6) + (value << 16) - value;
	}
};

template <> struct sdbm_hash<1>
{
	consteval static unsigned int _calculate(const char* const text, unsigned int&) { return text[0]; }
};

template <size_t N> consteval unsigned int make_sdbm_hash(const char(&text)[N])
{
	return sdbm_hash<N - 1>::calculate(text);
}