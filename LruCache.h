#pragma once
#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "CachePolicy.h"

namespace Cache{

template<typename Key, typename Value> class LruCache;

template<typename Key, typename Value> 
class LruNode{
private:
    Key key;
    Value value;
    size_t accessCount;
    std::shared_ptr<LruNode<Key, Value>> prev;
    std::shared_ptr<LruNode<Key, Value>> next;

public:
    LruNode(Key key_, Value value)
        : key(key)
        , value(value_)
        , accessCount(1)
        , prev(nullptr)
        , next(nullptr)
    {}

    Key getKey() const { return key; }
    Value getValue() const { return value; }
    void setValue(const Value& value) { value = value; }
    size_t getAcceseeCount() const { return accessCount; }
    void increasementAccessCount() { ++ accessCount; }

    friend class LruCache<Key, Value>; //让 LruCache 能够访问 LruNode 类的私有成员
};

template<typename Key, typename Value>
class LruCache : public CachePolicy<Key, Value> {
public:
    LruCache(int capacity_) : capacity(capacity_) {
        dummy = std::make_shared<LruNode<Key, Value>>(Key(), Value());
        dummy->prev = dummy;
        dummy->next = dummy;
    }


private:
    int capacity; //缓存容量
    std::unordered_map<Key, std::shared_ptr<LruNode<Key, Value>>> key_to_node;
    std::shared_ptr<LruNode<Key, Value>> dummy;
    std::mutex mtx;
};

}