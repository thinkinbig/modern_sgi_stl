#pragma once

#include <unordered_map>
#include <iostream>
#include <list>
#include <stdexcept>

namespace mstl {

    template <typename Key, typename Value>
    class LRUCache {
    private:
        struct Node {
            Key key;
            Value value;
            Node(Key key, Value value) : key(key), value(value) { }
        };
    public:
        using KeyType = Key;
        using ValueType = Value;
        using SizeType = size_t;
        using Reference = Value&;
        using ConstReference = const Value&;
        using Iterator = typename std::list<Node>::iterator;
        using ConstIterator = typename std::list<Node>::const_iterator;
        using Allocator = std::allocator<Node>;

    private:

        SizeType capacity_;
        std::list<Node> cache_;
        std::unordered_map<Key, typename std::list<Node>::iterator> map_;


    public:
        LRUCache(SizeType capacity): capacity_(capacity), cache_(), map_() { }

        Value get(Key key) {
            auto it = map_.find(key);
            if (it == map_.end()) {
                throw std::range_error("Key not found in cache");
            }

            // Move accessed node to front
            cache_.splice(cache_.begin(), cache_, it->second);
            return it->second->value;
        }

        void put(const Key& key, const Value& value) {
            auto it = map_.find(key);

            if (it != map_.end()) {
                // Update existing value in map_
                it->second->value = value;
                cache_.splice(cache_.begin(), cache_, it->second);
                return;
            }
            if (map_.size() >= capacity_) {
                // Remove LRU Item
                Node lru = cache_.back();
                map_.erase(lru.key);
                cache_.pop_back();
            }

            // Insert new Item at front
            cache_.push_front(Node(key, value));
            map_[key] = cache_.begin();
        }

        bool contains(Key key) {
            return map_.find(key) != map_.end();
        }
    };

}