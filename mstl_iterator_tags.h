#ifndef __MSTL_ITERATOR_TAGS_H
#define __MSTL_ITERATOR_TAGS_H

namespace mstl {
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};
struct contiguous_iterator_tag : public random_access_iterator_tag {};
}  // namespace mstl

#endif