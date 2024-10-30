#ifndef _COMPRESSED_VECTOR_HPP_
#define _COMPRESSED_VECTOR_HPP_

#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <execution>
#include <cmath>
#include <initializer_list>
#include <concepts>
#include <shared_mutex>
#include <string_view>
#include <span>
#include <cstring>
#include <fstream>

#define LOCK_READ(threadSafe) if constexpr ( threadSafe ) const std::shared_lock lock(this->mutex);

#define LOCK_WRITE(threadSafe) if constexpr ( threadSafe ) const std::unique_lock lock(this->mutex);


template<class T> concept Scalar = std::is_scalar_v<T>;

template<class T> concept BigInteger = std::is_same_v<T, int32_t> or std::is_same_v<T, uint32_t> or std::is_same_v<T, int64_t> or std::is_same_v<T, uint64_t>;

template<class keyType, class valueType> concept ArithmeticMethodTypes = std::is_arithmetic_v<valueType> and ( sizeof(valueType) >= sizeof(keyType) );


template<Scalar T, bool safetyOn = false>
inline T safeAdd(const T& a, const T& b)
{
    if constexpr ( safetyOn )
    {
        if ( b > ( T ) 0 && a > std::numeric_limits<T>::max() - b )
            return std::numeric_limits<T>::max();

        if ( b < ( T ) 0 && a < std::numeric_limits<T>::min() - b ) 
            return std::numeric_limits<T>::min();
        
        return a + b;
    }

    else
        return a + b;
}


template<Scalar T, bool safetyOn = false>
inline T safeSubstract(const T& a, const T& b)
{
    if constexpr ( safetyOn )
    {
        if ( b < ( T ) 0 && a > std::numeric_limits<T>::max() + b )
            return std::numeric_limits<T>::max();

        if ( b > ( T ) 0 && a < std::numeric_limits<T>::min() + b ) 
            return std::numeric_limits<T>::min();
        
        return a - b;
    }

    else
        return a - b;
}


template<Scalar T, bool safetyOn = false>
inline T safeMultiply(const T& a, const T& b)
{
    if constexpr ( safetyOn )
    {
        if constexpr ( std::numeric_limits<T>::is_integer )
        {
            if ( ( std::cmp_equal(a, -1) and std::cmp_equal(b, std::numeric_limits<T>::min()) ) or ( std::cmp_equal(b, -1) and std::cmp_equal(a, std::numeric_limits<T>::min()) ) )
                return ( ( T ) -1 ) * ( std::numeric_limits<T>::min() + ( T ) 1 );
        }
        
        if ( b not_eq ( T ) 0 and a > std::numeric_limits<T>::max() / b )
            return std::numeric_limits<T>::max();

        if ( b not_eq ( T ) 0 and a < std::numeric_limits<T>::min() / b )
            return std::numeric_limits<T>::min();
        
        return a * b;
    }

    else
        return a * b;
}


template<Scalar T, bool safetyOn = false>
inline T safeDivide(const T& a, const T& b)
{
    if constexpr ( safetyOn )
    {
        if constexpr ( std::numeric_limits<T>::is_integer )
        {
            if ( ( std::cmp_equal(a, -1) and std::cmp_equal(b, std::numeric_limits<T>::min()) ) )
                return ( ( T ) -1 ) / ( std::numeric_limits<T>::min() + ( T ) 1 );

            if ( ( std::cmp_equal(b, -1) and std::cmp_equal(a, std::numeric_limits<T>::min()) ) )
                return ( std::numeric_limits<T>::min() + 1 ) / ( T ) -1;
        }
                
        return a / b;
    }

    else
        return a / b;
}


template<Scalar T, bool safetyOn = false>
inline void safeAddAssign(const T& target, const T& argument)
{
    target = safeAdd<T, safetyOn>(target, argument);
}


template<Scalar T, bool safetyOn = false>
inline void safeSubstractAssign(const T& target, const T& argument)
{
    target = safeSubstract<T, safetyOn>(target, argument);
}


template<Scalar T, bool safetyOn = false>
inline void safeMultiplyAssign(const T& target, const T& argument)
{
    target = safeMultiply<T, safetyOn>(target, argument);
}


template<Scalar T, bool safetyOn = false>
inline void safeDivideAssign(const T& target, const T& argument)
{
    target = safeDivide<T, safetyOn>(target, argument);
}


template<bool threadSafe>
class compressedVectorBase {};


template<>
class compressedVectorBase<true>
{
    mutable std::shared_mutex mutex;
};


template <BigInteger keyType = uint64_t, class valueType = int64_t, bool arithmeticSafe = false, bool threadSafe = false>
class compressedVector : compressedVectorBase<threadSafe>
{
    keyType minIndex, maxIndex;

    valueType m_valBegin;

    std::map<keyType, valueType> m_map;

    /*
    using value_type             = T;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using size_type              = std::size_t;                 
    using difference_type        = std::ptrdiff_t;             
    using iterator               = // implementation-defined;
    using const_iterator         = // implementation-defined;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    */


    constexpr compressedVector(const keyType& maxArg, const std::map<keyType, valueType>& mapArg):
        minIndex{ mapArg.empty() ? maxArg - keyType(1) : mapArg.cbegin()->first }, maxIndex{ maxArg }, 
        m_valBegin{ mapArg.empty() ? valueType(0) : mapArg.crbegin()->second }, m_map{ mapArg } 
        {
            if ( not this->mapCanonicalValidationDebug() )
                this->reset(valueType(0));
        }


    /*constexpr compressedVector(const std::initializer_list<std::pair<const keyType, valueType>>& list):
        //compressedVector(std::empty(list) ? keyType(0) : std::min(list).first, std::empty(list) ? valueType(0) : std::crbegin(list)->second, list){}
        minIndex{ std::empty(list) ? keyType(0) : std::min(list).first }, maxIndex{ std::numeric_limits<keyType>::max() },
        m_valBegin{ std::empty(list) ? valueType(0) : std::crbegin(list)->second }, m_map{ list } {}*/


    const auto& getMapRef(void) const noexcept { return this->m_map; }


public:

    using value_type = valueType;

    using key_type = keyType;


    constexpr compressedVector():
        minIndex{ keyType(0) }, maxIndex{ keyType(1) }, m_valBegin{ valueType(0) }, m_map{} {}


    constexpr explicit compressedVector(const valueType& initValue):
        minIndex{ keyType(0) }, maxIndex{ keyType(1) }, m_valBegin{ initValue }, m_map{} {}


    constexpr compressedVector
    (
        const keyType& minIndexArg, 
        const keyType& maxIndexArg, 
        const valueType& initValue
    ):
        minIndex{ std::min(minIndexArg, maxIndexArg) }, maxIndex{ std::max(maxIndexArg, minIndexArg) }, m_valBegin{ initValue }, m_map{} {}


    constexpr compressedVector
    (
        keyType minIndexArg, 
        const valueType& initValue, 
        const std::vector<std::pair<valueType, std::size_t>>& vectorValues
    ):
        minIndex{ minIndexArg }, 
        maxIndex{ std::ranges::fold_left(vectorValues, minIndexArg, [](const keyType& left, const auto& right){ return left + ( keyType ) right.second; }) },
        m_valBegin{ initValue }, 
        m_map{}
    {
        this->assignValues(vectorValues);
    }


    constexpr compressedVector(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other):
        minIndex{ other.minIndex }, maxIndex{ other.maxIndex }, m_valBegin { other.m_valBegin }, m_map { other.m_map } {}


    constexpr compressedVector<keyType, valueType, threadSafe>& operator=(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other) & noexcept
    {
        this->minIndex = other.minIndex;

        this->maxIndex = other.maxIndex;

        this->m_valBegin = other.m_valBegin;

        this->m_map = other.m_map;

        return *this;
    }


    constexpr compressedVector(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>&& other):
        minIndex{ other.minIndex }, maxIndex{ other.maxIndex }, m_valBegin { other.m_valBegin }, m_map { std::move(other.m_map) } {}


    constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& operator=(compressedVector<keyType, valueType, arithmeticSafe, threadSafe>&& other) & noexcept
    {
        if ( this == &other )
        {
            this->minIndex = other.minIndex;

            this->maxIndex = other.maxIndex;

            this->m_valBegin = other.m_valBegin;

            this->m_map = std::move(other.m_map);
        }

        return *this;
    }


    constexpr void reset(const valueType& value)
    {
        this->m_valBegin = value;

        this->m_map.clear();
    }


    [[nodiscard]] constexpr const valueType& operator[](keyType const& key) const 
    {
        if( const auto& it = this->m_map.upper_bound(key); it not_eq this->m_map.cbegin() )
            return std::prev(it)->second;
        else 
            return this->m_valBegin;
    }


    [[nodiscard]] constexpr bool operator==(const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& other) const = default;
    

    [[nodiscard]] constexpr const auto& getInitialValue(void) const noexcept
    {
        return this->m_valBegin;
    }


    [[nodiscard]] constexpr std::size_t getMapSize(void) const noexcept 
    { 
        return this->m_map.size(); 
    }


    [[nodiscard]] constexpr std::size_t getByteSize(void) const noexcept
    {
        const auto size = 2 * sizeof(keyType) + sizeof(valueType) + sizeof(decltype(this->m_map)) + sizeof(typename decltype(this->m_map)::value_type) * this->m_map.size();

        if constexpr ( threadSafe )
            return size + sizeof(decltype(this->mutex));

        else
            return size;
    }


    [[nodiscard]] constexpr keyType getMinIndex(void) const noexcept 
    { 
        return this->minIndex;
    }


    [[nodiscard]] constexpr keyType getMaxIndex(void) const noexcept 
    { 
        return this->maxIndex;
    }


    [[nodiscard]] constexpr std::pair<keyType, keyType> getMinMaxIndex(void) const
    {
        return { this->minIndex, this->maxIndex };
    }


    [[nodiscard]] constexpr auto getLength(void) const noexcept
    {
        return ( std::size_t ) ( this->maxIndex - this->minIndex );
    }


    [[nodiscard]] constexpr std::pair<keyType, keyType> getValidRange
    (
        const keyType& from, 
        const keyType& to
    ) const
    {
        if ( to <= from )
            return { from, from };

        if ( to <= this->minIndex )
            return { this->minIndex, this->minIndex };

        if ( this->maxIndex <= from )
            return { this->maxIndex, this->maxIndex };

        return { this->minIndex <= from ? from : this->minIndex, to <= this->maxIndex ? to : this->maxIndex };
    }


    [[nodiscard]] constexpr bool contains
    (
        const keyType& fromArg, 
        const keyType& toArg,
        const valueType& value
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to ) 
            return false;

        if ( this->m_map.empty() )
            return this->m_valBegin == value;

        const auto itBegin = this->m_map.lower_bound(from);
        const auto itEnd = this->m_map.lower_bound(to);

        if ( itEnd == this->m_map.cbegin() )
            return this->m_valBegin == value;

        if ( itBegin == this->m_map.cend() )
            return this->m_map.crbegin()->second == value;

        const valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );

        return ( prevItBeginVal == value ) or ( std::find_if(itBegin, itEnd, 
            [&value](const std::pair<const keyType, valueType>& p) { return p.second == value; }) not_eq this->m_map.cend() );   
    }


    [[nodiscard]] constexpr bool contains(const valueType& value) const
    {
        return this->contains(this->minIndex, this->maxIndex, value);
    }


    [[maybe_unused]] constexpr bool setMinIndex(const keyType& index)
    {
        if ( index >= this->maxIndex )
            return false;

        this->minIndex = index;

        this->m_map.erase( this->m_map.cbegin(), this->m_map.lower_bound(index) );

        return true;
    }


    [[maybe_unused]] constexpr bool setMaxIndex(const keyType& index)
    {
        if ( index <= this->minIndex )
            return false;

        this->maxIndex = index;

        this->m_map.erase( this->m_map.upper_bound(index), this->m_map.cend() );

        return true;
    }


    [[maybe_unused]] constexpr bool setMinMaxIndex
    (
        const keyType& minIndexArg, 
        const keyType& maxIndexArg
    )
    {
        return this->setMinIndex(minIndexArg) and this->setMaxIndex(maxIndexArg);
    }


    constexpr void setInitValue(const valueType& value)
    {
        this->m_valBegin = value;

        if ( this->m_map.empty() )
            return;

        if ( value == this->m_map.cbegin()->second )
            this->m_map.erase(this->m_map.cbegin());

        if ( this->m_map.size() == 1 )
        {
            this->m_map.clear();

            return;
        }

        this->m_map.rbegin()->second = value;
    }


    constexpr void assignValue
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const valueType& value
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to ) 
            return;

        if ( m_map.empty() )
        {
            if ( value not_eq this->m_valBegin )
                this->m_map = { { from, value }, { to, this->m_valBegin } };

            return;
        }

        typename decltype(this->m_map)::iterator itBegin, itEnd = this->m_map.lower_bound(to);

        if ( itEnd == this->m_map.cend() )
        {
            if ( value not_eq this->m_valBegin )
                itEnd = this->m_map.emplace_hint(itEnd, to, this->m_valBegin);

            itBegin = std::prev(itEnd);
        }

        else
        {
            if ( itEnd == this->m_map.cbegin() )
            {
                if ( to < itEnd->first and value not_eq this->m_valBegin )
                    itEnd = this->m_map.emplace_hint(itEnd, to, this->m_valBegin);
            }

            else if ( const valueType& prevItEndVal = std::prev(itEnd)->second; to < itEnd->first and value not_eq prevItEndVal )
                itEnd = this->m_map.emplace_hint(itEnd, to, prevItEndVal);

            itBegin = itEnd;
        }

        for ( ; itBegin not_eq this->m_map.cbegin() and from < itBegin->first ; --itBegin );

        if ( itBegin->first < from )
            ++itBegin;

        const valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );

        if ( from < itBegin->first )
        {
            const auto& itEndErase = this->m_map.erase(itBegin, itEnd not_eq this->m_map.cend() and itEnd->second == value ? std::next(itEnd) : itEnd);
            
            if ( value not_eq prevItBeginVal )
                this->m_map.emplace_hint(itEndErase, from, value);
        }

        else
        {
            if ( value not_eq prevItBeginVal )
                (itBegin++)->second = value;
        
            this->m_map.erase(itBegin, itEnd not_eq this->m_map.cend() and itEnd->second == value ? std::next(itEnd) : itEnd);
        }
    }


    constexpr void assignValues(const std::vector<std::pair<valueType, std::size_t>>& values)
    {
        this->assignValues(this->minIndex, this->maxIndex, values);
    }


    constexpr void assignValues
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const std::vector<std::pair<valueType, std::size_t>>& values
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to or values.empty() ) 
            return;

        std::size_t startIndex { 0 }, reduceFirst { 0 };

        auto diff = from - fromArg;

        if ( std::cmp_greater(diff, 0) )
        {
            for ( const auto& item : values )
            {   
                diff -= ( keyType ) item.second;

                if ( std::cmp_less(diff, 0) )
                {
                    reduceFirst = item.second - ( std::size_t ) -diff ;

                    break;
                }

                ++startIndex;
            }
        }

        if ( std::cmp_equal(startIndex, values.size()) )
            return;

        std::vector<std::pair<valueType, std::size_t>> fixedValues = { values[startIndex] };

        fixedValues.front().second -= reduceFirst;

        const std::size_t rangeSize = ( std::size_t ) std::llround(std::ceil(to - from));

        std::size_t size { fixedValues.front().second };

        for ( std::size_t index { startIndex + 1 } ; index not_eq values.size() and size < rangeSize ; )
        {
            const auto& item = values[index++];

            if ( item.first not_eq fixedValues.back().first )
            {
                if ( item.second )
                    fixedValues.push_back(item);
            }

            else
                fixedValues.back().second += item.second;

            size += item.second;
        }

        if ( size > rangeSize )
        {
            fixedValues.back().second -= (size - rangeSize);

            size = rangeSize;
        }

        if ( std::cmp_equal(fixedValues.back().second, 0) )
            fixedValues.pop_back();

        if ( fixedValues.empty() )
            return;

        if ( std::cmp_equal(fixedValues.size(), 1) )
        {
            this->assignValue(from, from + ( keyType ) fixedValues.front().second, fixedValues.front().first);

            return;
        }

        if ( m_map.empty() )
        {
            keyType position = ( keyType ) from;

            for ( const auto& item : fixedValues )
            {
                this->m_map.emplace_hint(this->m_map.cend(), position, item.first);

                position += ( keyType ) item.second;
            }

            if ( ( not this->m_map.empty() ) and this->m_map.crbegin()->second not_eq this->m_valBegin )
                this->m_map.emplace_hint(this->m_map.cend(), position, this->m_valBegin);

            if ( ( not this->m_map.empty() ) and this->m_map.cbegin()->second == this->m_valBegin )
                this->m_map.erase(this->m_map.cbegin());

            return;
        }

        const auto positionTemp = from + ( keyType ) size - ( keyType ) fixedValues.back().second;

        auto position { positionTemp };

        this->assignValue(from, from + ( keyType ) fixedValues.front().second, fixedValues.front().first);

        this->assignValue(( keyType ) position, from + ( keyType ) size, fixedValues.back().first);

        if ( std::cmp_equal(fixedValues.size(), 2) )
            return;

        const auto secondPrevEndIt = std::prev(fixedValues.cend(), 2);

        position -= ( decltype(position) ) secondPrevEndIt->second;

        this->assignValue(( keyType ) position, positionTemp, secondPrevEndIt->first);

        if ( std::cmp_equal(fixedValues.size(), 3) )
            return;

        auto it = std::prev(this->m_map.find(positionTemp));

        this->m_map.erase(this->m_map.upper_bound(from), it);

        if ( std::cmp_not_equal(it->first, position) )
            it = this->m_map.emplace_hint(this->m_map.erase(it), position, secondPrevEndIt->first);

        for ( auto valuesIt { std::prev(fixedValues.cend(), 3) } ; valuesIt not_eq fixedValues.cbegin() ; --valuesIt )
        {
            position -= ( decltype(position) ) valuesIt->second;

            it = this->m_map.emplace_hint(it, position, valuesIt->first);
        }
        /* Maybe not needed */
        if ( ( not this->m_map.empty() ) and this->m_map.crbegin()->second not_eq this->m_valBegin )
            this->m_map.emplace_hint(this->m_map.cend(), position, this->m_valBegin);
        /* Maybe not needed */
        if ( ( not this->m_map.empty() ) and this->m_map.cbegin()->second == this->m_valBegin )
            this->m_map.erase(this->m_map.cbegin());
    }


    constexpr void assignRangeFromArray
    (
        compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, 
        const keyType& fromA, 
        const keyType& toA,
        const keyType& fromB, 
        const keyType& toB
    )
    {
        // TODO
    }


    constexpr void swapRangesWithArray
    (
        const keyType& fromA, 
        const keyType& toA, 
        compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, 
        const keyType& fromB, 
        const keyType& toB
    )
    {
        // TODO
    }


    constexpr void swapRanges
    (
        const keyType& fromA, 
        const keyType& toA, 
        const keyType& fromB, 
        const keyType& toB
    )
    {
        // TODO
    }


    constexpr void copyRange
    (
        const keyType& fromA, 
        const keyType& toA, 
        const keyType& fromB, 
        const keyType& toB
    )
    {
        // TODO
    }


    constexpr void insertValuesAfterPosition
    (
        const keyType& position, 
        const std::vector<std::pair<valueType, std::size_t>>& values,
        const bool expandLeft = false
    )
    {
        if ( position < this->minIndex or position >= this->maxIndex or values.empty() )
            return;

        std::vector<std::pair<valueType, std::size_t>> fixedValues = { values.front() };

        std::size_t size { fixedValues.front().second };

        for ( auto it = std::next(values.cbegin()) ; it not_eq values.cend() ; ++it )
        {
            if ( it->first not_eq fixedValues.back().first )
            {
                if ( it->second )
                    fixedValues.push_back(*it);
            }

            else
                fixedValues.back().second += it->second;

            size += it->second;
        }

        if ( this->m_map.empty() )
        {
            if ( fixedValues.back().first == this->m_valBegin )
                fixedValues.pop_back();

            auto index = position + 1 + ( expandLeft ? ( keyType ) -size : 0 );

            for ( const auto& [value, freq] : fixedValues )
            {
                this->m_map.emplace_hint(this->m_map.cend(), index, value);

                index += ( keyType ) freq;
            }

            this->m_map.emplace_hint(this->m_map.cend(), index, this->m_valBegin);

            if ( ( not this->m_map.empty() ) and this->m_map.cbegin()->second == this->m_valBegin )
                this->m_map.erase(this->m_map.cbegin());

            if ( expandLeft )
                this->minIndex -= ( keyType ) size;

            else
                this->maxIndex += ( keyType ) size;

            return;
        }

        auto itPosition = this->m_map.upper_bound(position);

        const auto nextKey = itPosition->first;

        auto prevVal = ( itPosition == this->m_map.cbegin() ? this->m_valBegin : std::prev(itPosition)->second );

        std::vector<typename decltype(this->m_map)::node_type> shifted;

        if ( expandLeft )
        {
            for ( auto it = this->m_map.begin(); it not_eq itPosition; )
            {
                shifted.push_back(this->m_map.extract(it++));

                shifted.back().key() -= ( keyType ) size;
            }

            auto index = position + 1 - ( keyType ) size;

            auto itNew = fixedValues.begin();

            if ( itNew->first == prevVal )
                index += ( keyType ) (itNew++)->second;

            for ( ; itNew not_eq fixedValues.cend(); ++itNew )
            {
                this->m_map.emplace_hint(itPosition, index, itNew->first);

                index += ( keyType ) itNew->second;
            }

            if ( std::prev(itPosition)->second not_eq prevVal and nextKey - position > 1 )
                this->m_map.emplace_hint(itPosition, index, prevVal);

            else if ( itPosition->second == fixedValues.back().first )
                this->m_map.erase(itPosition);

            itPosition = this->m_map.begin();

            if ( not shifted.empty() )
                for ( auto it = ( shifted.crbegin()->mapped() == itPosition->second ? std::next(shifted.rbegin()) : shifted.rbegin() ) ; it not_eq shifted.crend(); ++it )
                    this->m_map.insert(itPosition, std::move(*it));

            else if ( this->m_map.cbegin()->second == this->m_valBegin )
                this->m_map.erase(this->m_map.cbegin());

            if ( this->m_map.crbegin()->second not_eq this->m_valBegin )
                this->m_map.emplace_hint(this->m_map.cend(), index, this->m_valBegin);

            this->minIndex -= ( keyType ) size;
        }

        else
        {
            for ( auto it = itPosition; it not_eq this->m_map.cend(); )
            {
                shifted.push_back(this->m_map.extract(it++));

                shifted.back().key() += ( keyType ) size;
            }

            auto index = position + 1;

            auto itNew = fixedValues.begin();

            if ( itNew->first == prevVal )
                index += ( keyType ) (itNew++)->second;

            for ( ; itNew not_eq fixedValues.cend(); ++itNew )
            {
                this->m_map.emplace_hint(this->m_map.cend(), index, itNew->first);

                index += ( keyType ) itNew->second;
            }

            if ( this->m_map.crbegin()->second not_eq prevVal and nextKey - position > 1 )
                this->m_map.emplace_hint(this->m_map.cend(), index, prevVal);

            if ( not shifted.empty() )
                for ( auto it = ( shifted.cbegin()->mapped() == this->m_map.crbegin()->second ? std::next(shifted.begin()) : shifted.begin()); it not_eq shifted.cend(); ++it )
                    this->m_map.insert(this->m_map.cend(), std::move(*it));

            else if ( this->m_map.crbegin()->second not_eq this->m_valBegin )
                this->m_map.emplace_hint(this->m_map.cend(), index, this->m_valBegin);

            this->maxIndex += ( keyType ) size;
        }
    }


    constexpr void appendValues
    (
        const std::vector<std::pair<valueType, std::size_t>>& values, 
        const bool expandLeft = false
    )
    {
        if ( expandLeft )
        {
            const auto prevMinIndex = this->minIndex;

            this->minIndex = std::ranges::fold_left(values, this->minIndex, [](const keyType& left, const auto& right){ return left - ( keyType ) right.second; });

            const auto insertedSize = prevMinIndex - this->minIndex;

            std::vector<typename decltype(this->m_map)::node_type> nodes;
            nodes.reserve(this->m_map.size());

            for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); )
            {
                nodes.push_back(this->m_map.extract(it++));

                nodes.back().key() -= insertedSize;
            }

            for ( auto& node : nodes )
                this->m_map.insert(this->m_map.cend(), std::move(node));

            this->assignValues(this->maxIndex - insertedSize, this->maxIndex, values);

            return;
        }

        const auto prevMaxIndex = this->maxIndex;

        this->maxIndex = std::ranges::fold_left(values, this->maxIndex, [](const keyType& left, const auto& right){ return left + ( keyType ) right.second; });

        this->assignValues(prevMaxIndex, this->maxIndex, values);
    }


    constexpr void prependValues
    (
        const std::vector<std::pair<valueType, std::size_t>>& values, 
        const bool expandLeft = false
    )
    {
        if ( expandLeft )
        {
            const auto prevMinIndex = this->minIndex;

            this->minIndex = std::ranges::fold_left(values, this->minIndex, [](const keyType& left, const auto& right){ return left - ( keyType ) right.second; });

            this->assignValues(this->minIndex, prevMinIndex, values);

            return;
        }

        const auto prevMaxIndex = this->maxIndex;

        this->maxIndex = std::ranges::fold_left(values, this->maxIndex, [](const keyType& left, const auto& right){ return left + ( keyType ) right.second; });

        const auto insertedSize = this->maxIndex - prevMaxIndex;

        std::vector<typename decltype(this->m_map)::node_type> nodes;
        nodes.reserve(this->m_map.size());

        for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); )
        {
            nodes.push_back(this->m_map.extract(it++));

            nodes.back().key() += insertedSize;
        }

        for ( auto& node : nodes )
            this->m_map.insert(this->m_map.cend(), std::move(node));

        this->assignValues(this->minIndex, this->minIndex + insertedSize, values);
    }


    constexpr void erase
    (
        const keyType& fromArg, 
        const keyType& toArg,
        const bool shrinkRight = false
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to ) 
            return;

        const auto difference = to - from;

        auto itBegin = this->m_map.lower_bound(from);
        auto itEnd = this->m_map.lower_bound(to);

        std::vector<typename decltype(this->m_map)::node_type> shifted;
        shifted.reserve(std::distance(itBegin, itEnd));

        const auto prevItEndValue = ( itEnd not_eq this->m_map.cbegin() ? std::prev(itEnd)->second : this->m_valBegin );

        this->m_map.erase(itBegin, itEnd);

        if ( itBegin not_eq itEnd and itEnd not_eq this->m_map.cend() and to not_eq itEnd->first and prevItEndValue not_eq itEnd->second )
            this->m_map.emplace_hint(itEnd, to - difference, prevItEndValue);

        if ( shrinkRight )
        {
            for ( auto it = this->m_map.cbegin(); it not_eq itEnd; )
            {
                auto node = this->m_map.extract(it++);

                node.key() += difference;

                shifted.push_back(std::move(node));
            }

            if ( not shifted.empty() )
            {
                for ( auto& node : shifted )
                    this->m_map.insert(itEnd, std::move(node));

                if ( itEnd not_eq this->m_map.cend() and itEnd->second == std::prev(itEnd)->second )
                    this->m_map.erase(itEnd);
            }

            this->minIndex += difference;
        }

        else
        {
            for ( auto it = itEnd; it not_eq this->m_map.cend(); )
            {
                auto node = this->m_map.extract(it++);

                node.key() -= difference;

                shifted.push_back(std::move(node));
            }

            if ( not shifted.empty() )
            {
                const auto& lastValue = ( this->m_map.empty() ? this->m_valBegin : this->m_map.crbegin()->second );

                for ( auto it = ( shifted.cbegin()->mapped() not_eq lastValue ? shifted.begin() : std::next(shifted.begin()) ); it not_eq shifted.cend(); ++it )
                    this->m_map.insert(this->m_map.cend(), std::move(*it));
            }

            this->maxIndex -= difference;
        }

        if ( ( not this->m_map.empty() ) and this->m_map.cbegin()->second == this->m_valBegin )
            this->m_map.erase(this->m_map.cbegin());

        if ( ( not this->m_map.empty() ) and this->m_map.crbegin()->second not_eq this->m_valBegin )
            this->m_map.emplace_hint(this->m_map.cend(), from, this->m_valBegin);
    }


    constexpr void increaseValueRange
    (
        const keyType& from, 
        const keyType& to, 
        const valueType& value, 
        const std::size_t amount,
        const bool expandLeft = false
    )
    {
        // TODO
    }


    constexpr void decreaseValueRange
    (
        const keyType& from, 
        const keyType& to, 
        const valueType& value, 
        const std::size_t amount,
        const bool shrinkRight = false
    )
    {
        // TODO
    }


    constexpr void changeValueRange
    (
        const keyType& from, 
        const keyType& to, 
        const valueType& value, 
        const std::size_t newRange
    )
    {
        // TODO
    }


    constexpr void shiftLeft(const keyType& difference = keyType(1))
    {
        std::vector<typename decltype(this->m_map)::node_type> nodes;
        nodes.reserve(this->m_map.size());

        for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); )
        {
            nodes.push_back(this->m_map.extract(it++));

            nodes.back().key() -= difference;
        }

        for ( auto& node : nodes )
            this->m_map.insert(this->m_map.cend(), std::move(node));

        this->minIndex -= difference;

        this->maxIndex -= difference;
    }


    constexpr void shiftRight(const keyType& difference = keyType(1))
    {
        std::vector<typename decltype(this->m_map)::node_type> nodes;
        nodes.reserve(this->m_map.size());

        for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); )
        {
            nodes.push_back(this->m_map.extract(it++));

            nodes.back().key() += difference;
        }

        for ( auto& node : nodes )
            this->m_map.insert(this->m_map.cend(), std::move(node));

        this->minIndex += difference;

        this->maxIndex += difference;
    }


    constexpr void shiftRangeLeft
    (
        const keyType& from, 
        const keyType& to, 
        const keyType& difference = keyType(1), 
        const valueType& fillValue = valueType(0)
    )
    {
        // TODO
    }


    constexpr void shiftRangeRight
    (
        const keyType& from, 
        const keyType& to, 
        const keyType& difference = keyType(1), 
        const valueType& fillValue = valueType(0)
    )
    {
        // TODO
    }


    constexpr void reverseValues(void)
    {
        this->reverseValues(this->minIndex, this->maxIndex);
    }


    constexpr void reverseValues
    (
        const keyType& fromArg, 
        const keyType& toArg
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to or this->m_map.empty() )
            return;

        auto itBegin = this->m_map.lower_bound(from);
        auto itEnd = this->m_map.lower_bound(to);

        if ( itBegin == itEnd )
            return;

        const auto prevItBeginValue = ( itBegin not_eq this->m_map.cbegin() ? std::prev(itBegin)->second : this->m_valBegin );

        const bool insertExtraNode = from not_eq itBegin->first;

        if ( itEnd not_eq this->m_map.cend() and to not_eq itEnd->first )
            itEnd = this->m_map.emplace_hint(itEnd, to, std::prev(itEnd)->second);

        std::vector<typename decltype(this->m_map)::node_type> nodes;
        nodes.reserve(( std::size_t ) std::distance(itBegin, itEnd));

        for ( auto it = itBegin; it not_eq itEnd; )
            nodes.push_back(std::move(this->m_map.extract(it++)));

        auto currentIndex = from, prevIndex = to;

        auto it = nodes.rbegin();

        if ( it->mapped() == prevItBeginValue )
            currentIndex += ( to - ( prevIndex = (it++)->key() ) );

        for ( ; it not_eq nodes.rend(); ++it )
        {
            const auto newCurrentIndex = currentIndex + prevIndex - it->key();

            prevIndex = it->key();

            it->key() = currentIndex;

            currentIndex = newCurrentIndex;

            this->m_map.insert(itEnd, std::move(*it));
        }

        if ( insertExtraNode )
            this->m_map.emplace_hint(itEnd, currentIndex, prevItBeginValue);

        if ( itEnd not_eq this->m_map.cend() and itEnd->second == std::prev(itEnd)->second )
            this->m_map.erase(itEnd);

        if ( this->m_map.crbegin()->second not_eq this->m_valBegin )
            this->m_map.emplace_hint(this->m_map.cend(), to, this->m_valBegin);
    }


    constexpr void sortValues
    (
        const bool reverse = false,
        const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}
    )
    {
        this->sortValues(this->minIndex, this->maxIndex, reverse, compareFunction);
    }


    constexpr void sortValues
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const bool reverse = false,
        const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to or this->m_map.empty() )
            return;

        auto itBegin = this->m_map.lower_bound(from);
        auto itEnd = this->m_map.lower_bound(to);

        if ( itBegin == itEnd )
            return;

        const valueType prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );
        const valueType prevItEndVal = ( itEnd == this->m_map.cbegin() ? this->m_valBegin : std::prev(itEnd)->second );

        auto comparator = [reverse, &compareFunction](const auto& a, const auto& b) { return reverse ? compareFunction(b, a) : compareFunction(a, b); };

        std::map<valueType, std::size_t, decltype(comparator)> orderedValuesMap{ comparator };

        if ( itBegin->first not_eq from )
            orderedValuesMap[prevItBeginVal] += ( std::size_t ) ( itBegin->first - from );

        orderedValuesMap[ itEnd not_eq this->m_map.cend() ? std::prev(itEnd)->second : this->m_valBegin ] += ( std::size_t ) ( to - std::prev(itEnd)->first );

        for ( auto it = itBegin, itStop = std::prev(itEnd); it not_eq itStop; ++it )
            orderedValuesMap[it->second] += ( std::size_t ) ( std::next(it)->first - it->first );
        
        this->m_map.erase(itBegin, itEnd);

        auto itStart = orderedValuesMap.begin(), itStop = orderedValuesMap.end();
        auto currentKey = from;    

        if ( itStart->first == prevItBeginVal )
            currentKey += ( keyType ) (itStart++)->second;

        for ( auto& it = itStart; it not_eq itStop; ++it )
        {
            this->m_map.emplace_hint(itEnd, currentKey, it->first);

            currentKey += ( keyType ) it->second;
        }

        if ( to not_eq itEnd->first and std::prev(itEnd)->second not_eq prevItEndVal )
            this->m_map.emplace_hint(itEnd, to, prevItEndVal);
    }


    constexpr void replaceValue
    (
        const valueType& oldValue, 
        const valueType& newValue
    )
    {
        this->replaceValue(this->minIndex, this->maxIndex, oldValue, newValue);
    }


    constexpr void replaceValue
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const valueType& oldValue, 
        const valueType& newValue
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to or oldValue == newValue )
            return;

        if ( this->m_map.empty() )
        {
            if ( this->m_valBegin == oldValue )
                this->assignValue(from, to, newValue);

            return;
        }

        auto itBegin = this->m_map.lower_bound(from);
        auto itEnd = this->m_map.lower_bound(to);

        valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );
        valueType& prevItEndVal = ( itEnd == this->m_map.cbegin() ? this->m_valBegin : std::prev(itEnd)->second );

        if ( itBegin == itEnd )
        {
            if ( oldValue == prevItBeginVal )
                this->assignValue(from, to, newValue);

            return;
        }

        if ( from < itBegin->first and oldValue == prevItBeginVal )
        {
            this->m_map.emplace_hint(itBegin, from, newValue);

            if ( itBegin->second == newValue )
                itBegin = this->m_map.erase(itBegin);
        }

        if ( to < itEnd->first and oldValue == prevItEndVal )
        {
            this->m_map.emplace_hint(itEnd, to, prevItEndVal);

            if ( std::prev(itEnd)->second == itEnd->second )
                itEnd = this->m_map.erase(itEnd);
        }

        for ( auto it = itBegin ; it not_eq itEnd ; ++it )
            if ( it->second == oldValue and it->first not_eq to )
                it->second = newValue;

        if ( this->m_map.crbegin()->second not_eq this->m_valBegin )
            this->m_map.emplace_hint(this->m_map.cend(), this->maxIndex, this->m_valBegin);
    }


    constexpr void apply(const std::function<valueType(const valueType&)>& appliedFunction)
    {
        this->apply(this->minIndex, this->maxIndex, appliedFunction);
    }


    constexpr void apply
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const std::function<valueType(const valueType&)>& appliedFunction
    )
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return ;

        if ( this->m_map.empty() )
        {
            if ( const auto temp = appliedFunction(this->m_valBegin); temp not_eq this->m_valBegin )
                this->m_map = { { from, temp }, { to, this->m_valBegin } };

            return;
        }

        auto itBegin = this->m_map.lower_bound(from);
        const auto itEnd = this->m_map.lower_bound(to);

        const valueType prevItEndVal = ( itEnd == this->m_map.cbegin() ? this->m_valBegin : std::prev(itEnd)->second );

        if ( itBegin == itEnd )
        {
            if ( const auto newValue = appliedFunction(prevItEndVal); newValue not_eq prevItEndVal )
            {
                this->m_map.emplace_hint(itEnd, from, newValue);

                if ( to < itEnd->first )
                    this->m_map.emplace_hint(itEnd, to, prevItEndVal);

                else if ( itEnd->second == std::prev(itEnd)->second )
                    this->m_map.erase(itEnd);
            }

            return;
        }

        const valueType prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );

        bool itBeginNotChanged = true;

        if ( from < itBegin->first )
        {
            if ( const auto newValue = appliedFunction(prevItBeginVal); newValue not_eq prevItBeginVal )
            {
                itBegin = this->m_map.emplace_hint(itBegin, from, newValue);

                itBeginNotChanged = false;

                if ( std::next(itBegin)->second == newValue )
                    this->m_map.erase(std::next(itBegin));
            }
        }

        if ( itBeginNotChanged )
            itBegin->second = appliedFunction(itBegin->second);

        for ( auto it = std::next(itBegin); it not_eq itEnd; it = ( ( it->second = appliedFunction(it->second) ) == std::prev(it)->second ? this->m_map.erase(it) : std::next(it) ) );

        if ( itBegin->second == prevItBeginVal )
            this->m_map.erase(itBegin);

        if ( itEnd == this->m_map.cend() )
        {
            if ( this->m_map.crbegin()->second not_eq this->m_valBegin )
                this->m_map.emplace_hint(this->m_map.cend(), to, this->m_valBegin);

            return;
        }

        if ( to < itEnd->first )
        {
            if ( std::prev(itEnd)->second not_eq prevItEndVal )
                this->m_map.emplace_hint(itEnd, to, prevItEndVal);
        }

        else if ( std::prev(itEnd)->second == itEnd->second )
            this->m_map.erase(itEnd);
    }


    constexpr void operationWithArray
    (
        compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, 
        const keyType& fromArg, 
        const keyType& toArg, 
        const std::function<valueType(const valueType&, const valueType&)>& operation
    )
    {
        // TODO +-*/%&|^
    }


    [[nodiscard]] constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> getSubArray
    (
        const keyType& fromArg, 
        const keyType& toArg
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return {};

        if ( this->m_map.empty() or from >= this->m_map.crbegin()->first or to <= this->m_map.cbegin()->first )
            return { to, { { from, this->m_valBegin } } };

        const auto& itBegin = this->m_map.lower_bound(from);
        const auto& itEnd = this->m_map.lower_bound(to);

        const valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );

        if ( itBegin == itEnd )
            return { to, { { from, prevItBeginVal } } };

        std::map<keyType, valueType> resultMap;

        auto [mapHint, _] = resultMap.emplace(std::prev(itEnd)->first, std::prev(itEnd)->second);

        for ( auto it = std::prev(itEnd) ; it not_eq itBegin ; --it )
            mapHint = resultMap.emplace_hint(mapHint, std::prev(it)->first, std::prev(it)->second);

        resultMap.emplace_hint(mapHint, from, prevItBeginVal);

        return { to, resultMap };
    }


    [[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, valueType>> getValues(const bool shrink = false) const
    {
        this->getValues(this->minIndex, this->maxIndex, shrink);
    }


    [[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, valueType>> getValues
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const bool shrink = false
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to ) 
            return {};

        if ( this->m_map.empty() )
            return { { from, to, this->m_valBegin } };

        const auto itBegin = this->m_map.upper_bound(from);
        const auto itEnd = this->m_map.upper_bound(to);

        if ( itEnd == this->m_map.cbegin() or itBegin == this->m_map.cend() )
            return { { from, to, this->m_valBegin } };

        if ( itBegin == itEnd )
            return { { from, to, std::prev(itBegin)->second } };

        std::vector<std::tuple<keyType, keyType, valueType>> result = { { from, itBegin->first, itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second } };

        for ( auto it = std::next(itBegin) ; it not_eq itEnd ; ++it )
        {
            const auto& prevIt = std::prev(it);

            result.emplace_back(prevIt->first, it->first, prevIt->second);
        }

        if ( const auto& prevItEnd = std::prev(itEnd); prevItEnd->first not_eq to )
            result.emplace_back(prevItEnd->first, to, prevItEnd->second);

        if ( shrink )
            result.shrink_to_fit();

        return result;
    }


    [[nodiscard]] constexpr std::unordered_set<valueType> getSetValues(void) const
    {
        return this->getRangeSetValues(this->minIndex, this->maxIndex);
    }


    [[nodiscard]] constexpr std::unordered_set<valueType> getSetValues
    (
        const keyType& fromArg, 
        const keyType& toArg
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to ) 
            return {};

        if ( this->m_map.empty() )
            return { this->m_valBegin };

        const auto itBegin = this->m_map.upper_bound(from);
        const auto itEnd = this->m_map.upper_bound(to);

        if ( itEnd == this->m_map.cbegin() or itBegin == this->m_map.cend() )
            return { this->m_valBegin };

        if ( itBegin == itEnd )
            return { std::prev(itBegin)->second };

        std::unordered_set<valueType> result = { itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second };

        for ( auto it = std::next(itBegin) ; it not_eq itEnd ; ++it )
        {
            const auto& prevIt = std::prev(it);

            result.insert(prevIt->second);
        }

        if ( const auto& prevItEnd = std::prev(itEnd); prevItEnd->first not_eq to )
            result.insert(prevItEnd->second);

        return result;
    }


    [[nodiscard]] constexpr std::map<valueType, std::size_t> getRangeValueFrequencies(void) const
    {
        return this->getRangeValueFrequencies(this->minIndex, this->maxIndex);
    }


    [[nodiscard]] constexpr std::map<valueType, std::size_t> getRangeValueFrequencies
    (
        const keyType& fromArg, 
        const keyType& toArg
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        const auto subArray = this->getSubArray(from, to);
        subArray.printMapDebug();

        const auto& subArrayMapRef = subArray.getMapRef();

        if ( subArrayMapRef.empty() )
            return {};

        if ( subArrayMapRef.size() == 1 )
            return { { subArrayMapRef.cbegin()->second, to - from } };

        std::map<valueType, std::size_t> resultMap;

        for ( auto it = std::next(subArrayMapRef.begin()) ; it not_eq subArrayMapRef.cend() ; ++it )
            resultMap[std::prev(it)->second] += ( std::size_t ) ( it->first - std::prev(it)->first );

        resultMap[subArrayMapRef.crbegin()->second] += ( std::size_t ) ( to - subArrayMapRef.crbegin()->first );

        return resultMap;
    }


    [[nodiscard]] constexpr std::vector<std::pair<keyType, keyType>> getValueRanges
    (
        const valueType& value, 
        const bool shrink = false
    ) const
    {
        std::vector<std::pair<keyType, keyType>> result;

        if ( value == this->m_valBegin )
            result.emplace_back(this->minIndex, this->m_map.empty() ? this->maxIndex : this->m_map.cbegin()->first);

        for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); ++it )
        {
            if ( it->second == value )
            {
                const auto& itNext = std::next(it);

                result.emplace_back(it->first, itNext not_eq this->m_map.cend() ? itNext->first : this->maxIndex);

                ++it;
            }
        }

        if ( shrink )
            result.shrink_to_fit();

        return result;
    }


    [[nodiscard]] constexpr std::unordered_map<valueType, std::vector<std::pair<keyType, keyType>>> getValuesRanges
    (
        const std::unordered_set<valueType>& values, 
        const bool shrink = false
    ) const
    {
        std::unordered_map<valueType, std::vector<std::pair<keyType, keyType>>> result;

        if ( values.contains(this->m_valBegin) )
            result[this->m_valBegin].emplace_back(this->minIndex, this->m_map.empty() ? this->maxIndex : this->m_map.cbegin()->first);

        for ( auto it = this->m_map.cbegin(); it not_eq this->m_map.cend(); ++it )
        {
            if ( values.contains(it->second) )
            {
                const auto& itNext = std::next(it);

                result[it->second].emplace_back(it->first, itNext not_eq this->m_map.cend() ? itNext->first : this->maxIndex);
            }
        }

        if ( shrink )
            for ( auto& [value, vector] : result )
                vector.shrink_to_fit();

        return result;
    }


    [[nodiscard]] constexpr valueType getMinValue(const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}) const
    {
        return this->getMinValue(this->minIndex, this->maxIndex, compareFunction);
    }


    [[nodiscard]] constexpr valueType getMinValue
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less<valueType>{}
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return this->m_valBegin;

        const auto itBegin = this->m_map.lower_bound(from);

        if ( itBegin == this->m_map.cend() )
            return this->m_valBegin;

        const valueType& minFirst = ( itBegin->first == from ? itBegin->second : ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second ) );

        const auto itEnd = this->m_map.lower_bound(to);

        const valueType& minLast = ( itEnd == this->m_map.cend() or itEnd == this->m_map.cbegin() ? this->m_valBegin : std::prev(itEnd)->second );

        return std::min(
            std::min(minFirst, minLast, compareFunction), 
            std::min_element(std::execution::par_unseq, itBegin, itEnd, 
                [&compareFunction](const auto& itA, const auto& itB) { return compareFunction(itA.second, itB.second); }
                )->second, 
            compareFunction);
    }


    [[nodiscard]] constexpr valueType getMaxValue(const std::function<bool(const valueType&, const valueType&e)>& compareFunction = std::less<valueType>{}) const
    {
        return this->getMaxValue(this->minIndex, this->maxIndex, compareFunction);
    }


    [[nodiscard]] constexpr valueType getMaxValue
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::less{}
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return this->m_valBegin;

        const auto itBegin = this->m_map.lower_bound(from);

        if ( itBegin == this->m_map.cend() )
            return this->m_valBegin;

        const valueType& minFirst = ( itBegin->first == from ? itBegin->second : ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second ) );

        const auto itEnd = this->m_map.lower_bound(to);

        const valueType& minLast = ( itEnd == this->m_map.cend() or itEnd == this->m_map.cbegin() ? this->m_valBegin : std::prev(itEnd)->second );

        return std::max(
            std::max(minFirst, minLast, compareFunction), 
            std::max_element(std::execution::par_unseq, itBegin, itEnd, 
                [&compareFunction](const auto& itA, const auto& itB) { return compareFunction(itA.second, itB.second); }
                )->second, 
            compareFunction);
    }


    template<class = std::enable_if_t<ArithmeticMethodTypes<keyType, valueType>>>
    [[nodiscard]] constexpr valueType getSum(const valueType& init = valueType(0)) const
    {
        return this->getSum(this->minIndex, this->maxIndex, init);
    }


    template<class = std::enable_if_t<ArithmeticMethodTypes<keyType, valueType>>>
    [[nodiscard]] constexpr valueType getSum
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const valueType& init = valueType(0)
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return (*this)[from];

        if ( this->m_map.empty() or to < this->m_map.cbegin()->first or from > this->m_map.crbegin()->first )
            return init + this->m_valBegin * (to - from);

        const auto itBegin = this->m_map.lower_bound(from);
        const auto itEnd = this->m_map.lower_bound(to);

        const valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );
        
        if ( itBegin == itEnd )
            return init + (to - from) * prevItBeginVal;

        valueType result { init + ( itBegin->first - from ) * prevItBeginVal };

        for ( auto it = std::next(itBegin); it not_eq itEnd; ++it )
        {
            const auto& prevIt = std::prev(it);

            result += (it->first - prevIt->first) * prevIt->second;
        }

        const auto& prevItEnd = std::prev(itEnd);

        return result + (to - prevItEnd->first) * prevItEnd->second;
    }


    template<class = std::enable_if_t<ArithmeticMethodTypes<keyType, valueType>>>
    [[nodiscard]] constexpr valueType getProduct(const valueType& init = valueType(1)) const
    {
        return this->getProduct(this->minIndex, this->maxIndex, init);
    }


    template<class = std::enable_if_t<ArithmeticMethodTypes<keyType, valueType>>>
    [[nodiscard]] constexpr valueType getProduct
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        const valueType& init = valueType(1)
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return (*this)[from];

        if ( this->m_map.empty() or to < this->m_map.cbegin()->first or from > this->m_map.crbegin()->first )
            return init * ( valueType ) std::pow(this->m_valBegin, (to - from));

        const auto itBegin = this->m_map.lower_bound(from);
        const auto itEnd = this->m_map.lower_bound(to);

        const valueType& prevItBeginVal = ( itBegin == this->m_map.cbegin() ? this->m_valBegin : std::prev(itBegin)->second );
        
        if ( itBegin == itEnd )
            return init * ( valueType ) std::pow(prevItBeginVal, (to - from)) ;

        valueType result { init * ( valueType ) std::pow(prevItBeginVal, (itBegin->first - from)) };

        for ( auto it = std::next(itBegin); it not_eq itEnd; ++it )
        {
            const auto& prevIt = std::prev(it);

            result *= ( valueType ) std::pow(prevIt->second, (it->first - prevIt->first)) ;
        }

        const auto& prevItEnd = std::prev(itEnd);

        return result * ( valueType ) std::pow(prevItEnd->second, (to - prevItEnd->first));
    }


    [[nodiscard]] constexpr std::vector<std::tuple<keyType, keyType, bool>> compareWithArray
    (
        compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array, 
        const keyType& fromArgLeft, 
        const keyType& toArgLeft,
        const keyType& fromArgRight, 
        const keyType& toArgRight,
        const std::function<bool(const valueType&, const valueType&)>& compareFunction = std::equal_to<valueType>{}
    ) const
    {
        const auto [fromLeft, toLeft] = this->getValidRange(fromArgLeft, toArgLeft);
        const auto [fromRight, toRight] = array.getValidRange(fromArgRight, toArgRight);

        if ( fromLeft == toLeft or fromRight == toRight or ( toLeft -  fromLeft ) not_eq ( toRight - fromRight ) )
            return {};

        const auto valuesLeft = this->getValues(fromLeft, toLeft);
        const auto valuesRight = array.getValues(fromRight, toRight);
        // TODO
    }


    [[nodiscard]] constexpr std::vector<valueType> uncompressToVector(void) const
    {
        return this->uncompressToVector(this->minIndex, this->maxIndex);
    }


    [[nodiscard]] constexpr std::vector<valueType> uncompressToVector
    (
        const keyType& fromArg, 
        const keyType& toArg
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return {};

        const auto length = ( std::size_t ) std::llround(std::ceil(to - from));

        if ( constexpr std::vector<valueType> temp; length > temp.max_size() )
            return {};

        if ( this->m_map.empty() )
            return std::vector<valueType>(length , this->m_valBegin );

        const auto& itBegin = this->m_map.lower_bound(from);
        const auto& itEnd = this->m_map.lower_bound(to);

        if ( itEnd == this->m_map.cbegin() or itBegin == this->m_map.cend() )
            return std::vector<valueType>(length, this->m_valBegin );

        if ( itBegin == itEnd )
            return std::vector<valueType>(length, std::prev(itBegin)->second );

        std::vector<valueType> result;
        result.resize(length);

        std::size_t size { ( std::size_t ) std::llround(std::ceil(itBegin->first - from)) };

        auto itResult = result.begin();

        std::fill_n(std::execution::par_unseq, itResult, size, itBegin not_eq this->m_map.cbegin() ? std::prev(itBegin)->second : this->m_valBegin);

        std::advance(itResult, size);

        for ( auto it = next(itBegin) ; it not_eq itEnd ; ++it )
        {
            size = ( std::size_t ) ( std::llround(std::ceil(it->first - std::prev(it)->first)) );

            std::fill_n(std::execution::par_unseq, itResult, size, std::prev(it)->second);

            std::advance(itResult, size);
        }

        std::fill_n(std::execution::par_unseq, itResult, ( std::size_t ) ( std::llround(std::ceil(to - std::prev(itEnd)->first)) ), std::prev(itEnd)->second);

        return result;
    }


    void print
    (
        bool uncopressed = false, 
        std::ostream& os = std::cout, 
        const bool newLine = true, 
        const short prec = 3
    ) const
    {
        this->print(this->minIndex, this->maxIndex, uncopressed, os, newLine, prec);
    }


    void print
    (
        const keyType& fromArg, 
        const keyType& toArg, 
        bool uncopressed = false, 
        std::ostream& os = std::cout, 
        const bool newLine = true, 
        const short prec = 3
    ) const
    {
        const auto [from, to] = this->getValidRange(fromArg, toArg);

        if ( from == to )
            return;

        const auto currPrec = os.precision();

        constexpr bool isFloating = std::is_floating_point_v<keyType> or std::is_floating_point_v<valueType>;

        if constexpr ( isFloating )
            os.precision(prec);

        if ( newLine )
            os << "\n";

        if ( uncopressed )
        {
            for ( const auto item : uncompressToVector(from, to) )
                os << item << " ";

            os.flush();

            if constexpr ( isFloating )
                os.precision(currPrec);

            return;
        }

        if ( this->m_map.empty() )
        {
            os << this->minIndex << " | (" << this->m_valBegin << ", " << this->maxIndex - this->minIndex << ") | " << this->maxIndex;
            
            os.flush();

            if constexpr ( isFloating )
                os.precision(currPrec);

            return;
        }

        os << this->minIndex << " | ";

        const auto subArrayMap = this->getSubArray(from, to).m_map;
        
        for ( auto it = subArrayMap.cbegin(); it not_eq subArrayMap.cend(); ++it )
        {
            if ( std::next(it) not_eq subArrayMap.cend() )
                os << "(" << it->second << ", " << std::next(it)->first - it->first << "), ";

            else
                os << "(" << it->second << ", " << this->maxIndex - it->first << ")";
        }

        os << " | " << this->maxIndex;

        os.flush();

        if constexpr ( isFloating )
            os.precision(currPrec);
    }


    friend std::ostream& operator<<(std::ostream& os, const compressedVector<keyType, valueType, arithmeticSafe, threadSafe>& array)
    {
        array.print(array.getMinIndex(), array.getMaxIndex(), false, os);

        return os;
    }


    [[maybe_unused]] bool toFile
    (
        std::string_view filePath, 
        const keyType& from, 
        const keyType& to, 
        bool append = false, 
        bool uncopressed = false, 
        bool trimEdges = true, const short prec = 3
    ) const
    {
        std::ofstream ostrm(filePath, append ? std::ios::app : std::ios::out);

        if ( not ostrm.is_open() )
            return false;

        this->print(from, to, uncopressed, ostrm, trimEdges, false, prec);

        return true;
    }


    [[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compress
    (
        const std::span<const valueType> buffer, 
        keyType startIndex = keyType(0), 
        const valueType& initValue = valueType(0), 
        double* ratio = nullptr
    )
    {
        if ( buffer.empty() )
        {
            if ( ratio )
                *ratio = 0.0;

            return { startIndex, initValue, {} };
        }

        std::size_t count { 1 };

        valueType value { buffer.front() };

        std::vector<std::pair<valueType, std::size_t>> values;

        for ( auto it = std::next(buffer.begin()); it not_eq buffer.end(); ++it )
        {
            if ( *it == value )
                ++count;

            else
            {
                values.emplace_back(value, count);

                value = *it;

                count = 1;
            }
        }

        values.emplace_back(value, count);

        compressedVector<keyType, valueType, arithmeticSafe, threadSafe> resultArray { startIndex, initValue, values };

        if ( ratio )
            *ratio = ( double ) resultArray.getByteSize() / ( double ) ( buffer.size() * sizeof(valueType) );

        return resultArray;
    }


    [[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compressFromFile
    (
        std::string_view filePath, 
        const std::size_t indexFromStart = 0, 
        const std::size_t indexFromEnd = 0, 
        const valueType& initValue = valueType(0), 
        double* ratio = nullptr
    )
    {
        std::ifstream file(filePath.data());

        if ( not file.is_open() )
            return {};

        std::size_t total { 0 };

        valueType number;

        while(file >> number)
            ++total;

        if ( total == 0 )
            return {};

        file.clear();
        file.seekg(0);

        const auto endIndex = total - indexFromEnd, length = endIndex - indexFromStart;

        std::vector<valueType> values;
        values.reserve(length);

        for ( std::size_t index { 0 }; index < indexFromStart; ++index )
            file >> number;

        for ( std::size_t index { indexFromStart }; index < endIndex; ++index )
        {
            file >> number;

            values.emplace_back(number);
        }

        return compressedVector<keyType, valueType, arithmeticSafe, threadSafe>::compress(values, 0, initValue, ratio);
    }


    [[nodiscard]] static constexpr compressedVector<keyType, valueType, arithmeticSafe, threadSafe> compressFromFileParts
    (
        std::string_view filePath,
        const std::vector<std::pair<const std::size_t, const std::size_t>>& ranges,
        const valueType& initValue = valueType(0), 
        double* ratio = nullptr
    )
    {
        if ( ranges.empty() )
            return {};

        std::ifstream file(filePath.data());

        if ( not file.is_open() )
            return {};

        std::size_t total { 0 };

        valueType number;

        while(file >> number)
            ++total;

        if ( total == 0 )
            return {};

        file.clear();
        file.seekg(0);

        std::vector<std::pair<std::size_t, std::size_t>> rangesFixedTemp, rangesFixed;
        rangesFixedTemp.reserve(ranges.size());
        rangesFixed.reserve(ranges.size());

        std::copy_if(std::execution::par_unseq, ranges.begin(), ranges.end(), std::back_inserter(rangesFixedTemp), [&total](const auto& p) { return p.first < p.second and p.second <= total; });

        if ( rangesFixedTemp.empty() )
            return {};

        std::sort(std::execution::par_unseq, rangesFixedTemp.begin(), rangesFixedTemp.end());

        rangesFixed.emplace_back(rangesFixedTemp.front());

        for ( auto it = std::next(rangesFixedTemp.cbegin()); it not_eq rangesFixedTemp.cend(); ++it )
            if ( it->first >= rangesFixed.back().second )
                rangesFixed.emplace_back(*it);

        rangesFixedTemp.clear();

        std::size_t valueIndex { 0 }, rangeIndex { 0 }, from, to;

        std::tie(from, to) = rangesFixed[rangeIndex];

        std::vector<valueType> values;

        while ( file >> number )
        {
            if ( from <= valueIndex and valueIndex < to )
                values.emplace_back(number);

            else if ( to <= valueIndex )
            {
                std::tie(from, to) = rangesFixed[++rangeIndex];

                if ( from <= valueIndex and valueIndex < to )
                    values.emplace_back(number);
            }

            ++valueIndex;
        }

        return compressedVector<keyType, valueType, arithmeticSafe, threadSafe>::compress(values, 0, initValue, ratio);
    }


    [[nodiscard]] bool mapCanonicalValidationDebug(void) const 
    {
        return 
        this->m_map.size() not_eq 1
        and 
        ( this->m_map.empty() or ( this->m_valBegin == this->m_map.crbegin()->second and this->m_valBegin not_eq this->m_map.cbegin()->second ) )
        and
        std::adjacent_find(std::execution::par_unseq, this->m_map.cbegin(), this->m_map.cend(), 
            [](const auto& a, const auto& b){ return a.second == b.second; }) == this->m_map.cend();
    }


    void printMapDebug(void) const
    {
        std::cout << std::endl;

        if ( this-> m_map.empty() )
            std::cout << "Map is empty.";

        for ( const auto& [key, value] : this->m_map )
            std::cout << "(" << key << ", " << value << "), ";
    }
};

#endif
