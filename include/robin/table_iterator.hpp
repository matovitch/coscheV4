#pragma once

#include "table_bucket.hpp"

#include <iterator>

namespace robin
{

template <class>
class TTable;

namespace table
{

template <class>
class TIterator;

template <class IteratorTraits>
bool operator==(const TIterator<IteratorTraits>& lhs,
                const TIterator<IteratorTraits>& rhs);

template <class IteratorTraits>
bool operator!=(const TIterator<IteratorTraits>& lhs,
                const TIterator<IteratorTraits>& rhs);

namespace iterator
{

template <class TableTraits>
struct TTraits
{
    using Table  = TTable<TableTraits>;
    using Type   = typename TableTraits::Type;
    using Bucket = typename TableTraits::Bucket;

    using Traits = TTraits<TableTraits>;

    using Iterator = TIterator<Traits>;
};

template <class TableTraits>
using TMakeFromTraits = TIterator<TTraits<TableTraits>>;

} // namespace iterator

template <class IteratorTraits>
class TIterator : std::iterator<std::forward_iterator_tag,
                                typename IteratorTraits::Type>
{
    using Bucket   = typename IteratorTraits::Bucket;
    using Table    = typename IteratorTraits::Table;
    using Type     = typename IteratorTraits::Type;
    using Iterator = typename IteratorTraits::Iterator;

    friend Table;

    friend bool operator==<IteratorTraits>(const Iterator& lhs,
                                           const Iterator& rhs);

    friend bool operator!=<IteratorTraits>(const Iterator& lhs,
                                           const Iterator& rhs);
public:

    TIterator(Bucket* const bucketPtr) :
        _bucketPtr{bucketPtr}
    {}

    TIterator(const Iterator& toCopy) :
        _bucketPtr{toCopy._bucketPtr}
    {}

    void operator=(const Iterator& toCopy)
    {
        _bucketPtr = toCopy._bucketPtr;
    }

    const Type& operator*() const
    {
        return _bucketPtr->value();
    }

    Iterator& operator++()
    {
        do
        {
            _bucketPtr++;

        } while (_bucketPtr->isEmpty());
        // we skipped the empty buckets ! Hoora !
        return *this;
    }

    Iterator operator++(int)
    {
        Iterator tmp{*this};
        operator++();
        return tmp;
    }

private:

    Bucket* _bucketPtr;
};

template <class IteratorTraits>
bool operator==(const TIterator<IteratorTraits>& lhs,
                const TIterator<IteratorTraits>& rhs)
{
    return lhs._bucketPtr == rhs._bucketPtr;
}

template <class IteratorTraits>
bool operator!=(const TIterator<IteratorTraits>& lhs,
                const TIterator<IteratorTraits>& rhs)
{
    return lhs._bucketPtr != rhs._bucketPtr;
}

} // namespace table

} // namespace robin
