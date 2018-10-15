#pragma once

#include "robin/table_iterator.hpp"
#include "robin/table_bucket.hpp"
#include "robin/xor_shifter.hpp"

#include "robin/buffer_manager.hpp"
#include "robin/likelyhood.hpp"

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

    static constexpr std::size_t MASK              = TableTraits::MASK;
    static constexpr std::size_t STACK_SIZE        = TableTraits::STACK_SIZE;
    static constexpr std::size_t LOAD_FACTOR_LEVEL = TableTraits::LOAD_FACTOR_LEVEL;

    static constexpr auto REHASH_PREV = BufferManager::VIEW_PREV;
    static constexpr auto REHASH_NEXT = BufferManager::VIEW_NEXT;

public:

    using       iterator = typename TableTraits::     Iterator;
    using const_iterator = typename TableTraits::ConstIterator;

    friend       iterator;
    friend const_iterator;

    TTable() :
        _mask{MASK},
        _size{0},
        _capacity{STACK_SIZE}
    {
        const auto& bufferView = _bufferManager.makeView();

        _buckets  = reinterpret_cast<Bucket*>(bufferView.data);
        _capacity =                           bufferView.size - 1;

        init();
    }

    template <class... Args>
    void emplace(Args&&... args)
    {
        Type t{args...};

        REDO: // goto label

        uint8_t dib = Bucket::FILLED;

        Bucket* head = _buckets + (_hasher(t) & _mask);

        // Skip filled buckets with larger dib
        while (dib < head->dib())
        {
            BUCKET_SCAN: // goto label

            ++dib;
            head = (++head == _endPtr) ? _buckets : head;

            if (ROBIN_UNLIKELY(!dib))
            {
                rehash<REHASH_NEXT>();
                goto REDO;
            }
        }

        if (head->isEmpty())
        {
            if (ROBIN_UNLIKELY(++_size << LOAD_FACTOR_LEVEL > (_capacity << LOAD_FACTOR_LEVEL) - _capacity))
            {
                rehash<REHASH_NEXT>();
                goto REDO;
            }

            head->fill(dib, std::move(t));

            if (_beginPtr == _endPtr)
            {
                _beginPtr = head;
            }
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

        Bucket* prec = _buckets + (_hasher(t) & _mask);

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !_comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == _endPtr) ? _buckets : prec;
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
        return iterator{_beginPtr, *this};
    }

    const_iterator begin() const
    {
        return const_iterator{_beginPtr, *this};
    }

    iterator end()
    {
        return iterator{_endPtr, *this};
    }

    const_iterator end() const
    {
        return const_iterator{_endPtr, *this};
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

    void erase(const_iterator it)
    {
        Bucket* bucketPtr = it._bucketPtr;

        shiftBuckets(bucketPtr);
    }

private:

    template <auto REHASH_TYPE>
    void rehash()
    {
        const Bucket* const oldBuckets  = _buckets;
        const std::size_t   oldCapacity = _capacity;

        const auto& bufferView = _bufferManager.template makeView<REHASH_TYPE>();

        _buckets = reinterpret_cast<Bucket*>(bufferView.data);

        if constexpr(REHASH_TYPE == REHASH_PREV)
        {
            _capacity >>= 2;
        }

        if constexpr (REHASH_TYPE == REHASH_NEXT)
        {
            _capacity <<= 1;
        }

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

    void shiftBuckets(Bucket* prec)
    {
        Bucket* succ = (prec + 1 == _endPtr) ? _buckets : prec + 1;

        // Shift the right-adjacent buckets to the left
        while (succ->dib() > Bucket::FILLED)
        {
            prec->fill(succ->dib() - 1, std::move(succ->value()));
            prec = succ;
            succ = (++succ == _endPtr) ? _buckets : succ;
        }

        // Empty the bucket and decrement the size
        prec->markEmpty();

        if (ROBIN_UNLIKELY(--_size << LOAD_FACTOR_LEVEL < ((_capacity << LOAD_FACTOR_LEVEL) - _capacity) >> 2 && _size > STACK_SIZE))
        {
            rehash<REHASH_PREV>();
            return;
        }

        if (prec == _beginPtr)
        {
            if (_size == 0)
            {
                _beginPtr = _endPtr;
                return;
            }

            do
            {
                _beginPtr = _buckets + (_xorShifter() & _mask);

            } while (_beginPtr->isEmpty());
        }
    }

    template <class Table, class Iterator, class Type>
    static Iterator tFind(Table& table, const Type& t)
    {
        uint8_t dib =  Bucket::FILLED;

        Bucket* prec = table._buckets + (table._hasher(t) & table._mask);

        const Comparator& comparator = table._comparator;
        Bucket* const     buckets    = table._buckets;
        Bucket* const     endPtr     = table._endPtr;

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == endPtr) ? buckets : prec;
        }

        if (dib == prec->dib())
        {
            return Iterator{prec, table};
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
            new (reinterpret_cast<void*>(_buckets + bucketIndex)) Bucket{};
        }

        _mask = _capacity - 1;

        _endPtr   = _buckets + _capacity;
        _beginPtr = _endPtr;

        _endPtr->markFilled(); // for it == end() optimization

        _size = 0;
    }

    Bucket*       _buckets;
    std::size_t   _mask;
    Bucket*       _endPtr;
    Bucket*       _beginPtr;
    Hasher        _hasher;
    Comparator    _comparator;
    std::size_t   _size;
    std::size_t   _capacity;
    XorShifter    _xorShifter;
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
