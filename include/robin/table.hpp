#pragma once

#include "robin/table_iterator.hpp"
#include "robin/table_bucket.hpp"

#include "robin/buffer_manager.hpp"
#include "robin/likelyhood.hpp"

#include <iostream>

namespace robin
{

template <class TableTraits>
class TTable
{
    using Type          = typename TableTraits::Type;
    using Bucket        = typename TableTraits::Bucket;
    using Hasher        = typename TableTraits::Hasher;
    using Comparator    = typename TableTraits::Comparator;
    using BufferManager = typename TableTraits::BufferManager;

    static constexpr std::size_t MASK = TableTraits::MASK;

    static constexpr const std::size_t LOAD_FACTOR_LEVEL = TableTraits::LOAD_FACTOR_LEVEL;

public:

    using       iterator = typename TableTraits::     Iterator;
    using const_iterator = typename TableTraits::ConstIterator;

    TTable() :
        _mask{MASK},
        _size{0}
    {
        const auto& bufferView = _bufferManager.makeView();

        _buckets  = reinterpret_cast<Bucket*>(bufferView.data);
        _capacity =                           bufferView.size - 1;

        init();
    }

    void rehash()
    {
        const Bucket* const oldBuckets  = _buckets;
        const std::size_t   oldCapacity = _capacity;

        const auto& bufferNextView = _bufferManager.makeNextView();

        _buckets = reinterpret_cast<Bucket*>(bufferNextView.data);

        _capacity <<= 1;
        _mask = _capacity - 1;
        _size = 0;

        init();

        for (std::size_t bucketIndex = 0; 
                         bucketIndex < oldCapacity;
                         bucketIndex++)
        {
            const auto& oldBucket = oldBuckets[bucketIndex];
            
            if (oldBucket.isFilled())
            {
                emplace(oldBucket.value());
                oldBucket.~Bucket();
            }
        }

        oldBuckets[oldCapacity].~Bucket();

        _bufferManager.swapBuffers();
    }

    template <class... Args>
    void emplace(Args&&... args)
    {
        Type t{args...};

        REDO: // goto label

        const std::size_t hash = _hasher(t);

        uint8_t dib = Bucket::FILLED;

        Bucket* head = _buckets + ((hash + dib) & _mask);

        // Skip filled buckets with larger dib
        while (dib < head->dib())
        {
            BUCKET_SCAN: // goto label

            ++dib;
            head = (++head == _buckets + _capacity) ? _buckets : head;

            if (ROBIN_UNLIKELY(!dib))
            {
                rehash();
                goto REDO;
            }
        }

        if (head->isEmpty())
        {
            if (ROBIN_UNLIKELY(++_size << LOAD_FACTOR_LEVEL > (_capacity << LOAD_FACTOR_LEVEL) - _capacity))
            {
                rehash();
                goto REDO;
            }

            head->fill(dib, std::move(t));
        }   
        else
        {
            if (dib != head->dib())
            {
                // Copy the value of the found bucket and insert our own
                Type tTmp = std::move(head->value());
                const uint8_t dibTmp = head->dib();

                head->fill(dib, std::move(t));

                t = std::move(tTmp);
                dib = dibTmp;
                goto BUCKET_SCAN;
            }
            else if (_comparator(t, head->value()))
            {
                return;
            }

            goto BUCKET_SCAN;
        }
    }

    void erase(const Type& t)
    {
        uint8_t dib = Bucket::FILLED;

        Bucket* prec = _buckets + ((_hasher(t) + dib) & _mask);

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !_comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == _buckets + _capacity) ? _buckets : prec;
        }

        if (dib == prec->dib())
        {
            shiftBuckets(prec);
        }
    }

    std::size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    iterator begin()
    {
        return tBegin<TTable<TableTraits>, iterator>(*this);
    }

    const_iterator begin() const
    {
        return tBegin<const TTable<TableTraits>, const_iterator>(*this);
    }

    iterator end()
    {
        return iterator{_buckets + _capacity};
    }

    const_iterator end() const
    {
        return const_iterator{_buckets + _capacity};
    }

    template <class Type>
    iterator find(const Type& t)
    {
        return tFind<TTable<TableTraits>, iterator, Type>(*this, t);
    }

    template <class Type>
    const_iterator find(const Type& t) const
    {
        return tFind<const TTable<TableTraits>, const_iterator, Type>(*this, t);
    }

    iterator erase(const_iterator it)
    {
        Bucket* bucketPtr = it._bucketPtr;

        shiftBuckets(bucketPtr);

        while (bucketPtr->isEmpty())
        {
            bucketPtr++;
        }

        return iterator{bucketPtr};
    }

private:

    void shiftBuckets(Bucket* prec)
    {
        Bucket* succ = (prec + 1 == _buckets + _capacity) ? _buckets : prec + 1;

        // Shift the right-adjacent buckets to the left
        while (succ->dib() > Bucket::FILLED)
        {
            prec->fill(succ->dib() - 1, std::move(succ->value()));
            prec = succ;
            succ = (++succ == _buckets + _capacity) ? _buckets : succ;
        }

        // Empty the bucket and decrement the size
        prec->markEmpty();

        _size--;
    }

    template <class Table, class Iterator>
    static Iterator tBegin(Table& table)
    {
        Bucket* bucketPtr = table._buckets;

        while (bucketPtr->isEmpty())
        {
            bucketPtr++;
        }

        return Iterator{bucketPtr};
    }

    template <class Table, class Iterator, class Type>
    static Iterator tFind(Table& table, const Type& t)
    {
        uint8_t dib =  Bucket::FILLED;

        Bucket* prec = table._buckets + ((table._hasher(t) + dib) & table._mask);

        const Comparator& comparator = table._comparator;
        const std::size_t capacity   = table._capacity;
        Bucket* const     buckets    = table._buckets;

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == buckets + capacity) ? buckets : prec;
        }

        if (dib == prec->dib())
        {
            return Iterator{prec};
        }

        // No luck :(
        return table.end();
    }


    void init()
    {
        for (std::size_t bucketIndex = 0; 
                 bucketIndex <= _capacity;
                 bucketIndex++)
        {
            new (reinterpret_cast<void*>(_buckets + bucketIndex)) Bucket();
        }

        _buckets[_capacity].markFilled(); // for it == end() optimization
    }

    Bucket*       _buckets;
    std::size_t   _mask;
    std::size_t   _size;
    std::size_t   _capacity;
    Hasher        _hasher;
    Comparator    _comparator;
    BufferManager _bufferManager;
};

namespace table
{

template <class TableType,
          class TableHasher,
          class TableComparator,
          std::size_t STACK_SIZE_LOG2,
          std::size_t TABLE_LOAD_FACTOR_LEVEL>
struct TTraits
{
    static constexpr std::size_t STACK_SIZE = 1 << STACK_SIZE_LOG2;
    static constexpr std::size_t MASK       = STACK_SIZE - 1;

    static constexpr std::size_t LOAD_FACTOR_LEVEL = TABLE_LOAD_FACTOR_LEVEL;

    using Type       = TableType;
    using Hasher     = TableHasher;
    using Comparator = TableComparator;

    using Bucket = TBucket<Type>;

    using BufferManager = buffer::manager::TMakeFromType<Bucket, STACK_SIZE>;

    using Traits = TTraits<Type, Hasher, Comparator, STACK_SIZE_LOG2, LOAD_FACTOR_LEVEL>;

    using      Iterator = table::iterator::TMakeFromTraits<Traits>;
    using ConstIterator = table::iterator::TMakeFromTraits<Traits>;   
};

} // namespace table

} // namespace robin
