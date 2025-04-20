#ifndef __MSTL_ITERATOR_TAGS_H
#define __MSTL_ITERATOR_TAGS_H

namespace mstl {
struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};
struct ContiguousIteratorTag : public RandomAccessIteratorTag {};
}  // namespace mstl

#endif