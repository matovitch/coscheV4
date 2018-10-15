#pragma once

#include "robin/table.hpp"

#include <unordered_set>
#include <functional>

namespace cosche
{

template <class>
class TGraph;

template <class Type>
class TNode
{
    template <class>
    friend class TGraph;

private:

    using NodeSetTraits = robin::table::TTraits<TNode<Type>*, std::hash<TNode<Type>*>, std::equal_to<TNode<Type>*>, 2, 4>;
    using NodeSet       = robin::TTable<NodeSetTraits>; /*std::unordered_set<TNode<Type>*>*/

    NodeSet _dependers;
    NodeSet _dependees;

public:

    template <class... ARGS>
    TNode(ARGS&&... args) : value{args...} {}

    Type value;
};

} // namespace cosche
