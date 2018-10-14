#pragma once

#include "cosche/likelyhood.hpp"
#include "cosche/singleton.hpp"
#include "cosche/factory.hpp"
#include "cosche/node.hpp"

#include <unordered_set>
#include <cstddef>

namespace cosche
{

template <class GraphType>
class TGraph
{

public:

    using Type = GraphType;
    using Node = TNode<Type>;

    TGraph() :
        _nodeFactory{TSingleton<NodeFactory>::instance()}
    {}

    template <class... ARGS>
    Node& makeNode(ARGS&&... args)
    {
        Node& node = _nodeFactory.make(args...);

        _pendings.emplace(&node);

        return node;
    }

    void attach(Node& lhs,
                Node& rhs)
    {
        lhs._dependees.emplace(&rhs);
        rhs._dependers.emplace(&lhs);

        block(lhs);
    }

    void attachBatch(Node& node, const std::vector<Node*>& dependees)
    {
        if (COSCHE_UNLIKELY(dependees.empty()))
        {
            return;
        }

        for (const auto dependee : dependees)
        {
            node     ._dependees.emplace(dependee);
            dependee->_dependers.emplace(&node);
        }

        block(node);
    }

    template <std::size_t BATCH_SIZE>
    void attachBatch(Node& node, Node* const (&dependees)[BATCH_SIZE])
    {
        for (std::size_t i = 0; i < BATCH_SIZE; i++)
        {
            Node* const dependee = dependees[i];
            node     ._dependees.emplace(dependee);
            dependee->_dependers.emplace(&node);
        }

        block(node);
    }

    void detach(Node& lhs,
                Node& rhs)
    {
        lhs._dependees.erase(&rhs);
        rhs._dependers.erase(&lhs);

        if (lhs._dependees.empty())
        {
            const auto& fit = _blockeds.find(&lhs);

            if (COSCHE_LIKELY(fit != _blockeds.end()))
            {
                _blockeds.erase(fit);
                _pendings.emplace(&lhs);
            }
        }
    }

    void detachAll(Node& node)
    {
        for (Node* const dependee : node._dependers)
        {
            auto&& _dependees = dependee->_dependees;

            _dependees.erase(&node);

            if (_dependees.empty())
            {
                _blockeds.erase(dependee);
                _pendings.emplace(dependee);
            }
        }
    }


    bool isCyclic()
    {
        return _pendings.empty() && !_blockeds.empty();
    }

    std::vector<Node*> makeCycle()
    {
        if (!isCyclic())
        {
            return {};
        }

        std::vector<Node*> cycle;

        Node* node = *(_blockeds.begin());

        do
        {
            cycle.push_back(node);
            node = *(node->_dependees.begin());
        }
        while (node != cycle.front());

        return cycle;
    }

    Node& top()
    {
        return **(_pendings.begin());
    }

    bool empty() const
    {
        return _pendings.empty();
    }

    void pop()
    {
        const auto& topIt = _pendings.begin();

        Node* const top = *topIt;

        detachAll(*top);

        _nodeFactory.recycle(top);

        _pendings.erase(top);
    }

private:

    void block(Node& node)
    {
        const auto& fit = _pendings.find(&node);

        if (COSCHE_LIKELY(fit != _pendings.end()))
        {
            _pendings.erase(fit);
            _blockeds.emplace(&node);
        }
    }

    struct NodePtrHasher
    {
        std::size_t operator()(const Node* const nodePtr) const
        {
            return (reinterpret_cast<std::size_t>(nodePtr) >> 8) * 11400714819323198485llu;
        }
    };


    using NodePtrSetTraits = robin::table::TTraits<Node*, NodePtrHasher, std::equal_to<Node*>, 3, 4>;
    using NodePtrSet       = robin::TTable<NodePtrSetTraits>;

    NodePtrSet _pendings;
    NodePtrSet _blockeds;

    using NodeFactory = TFactory<Node>;

    NodeFactory& _nodeFactory;
};

} // namespace cosche