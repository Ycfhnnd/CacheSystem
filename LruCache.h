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
    LruNode(Key key_, Value value_)
        : key(key_)
        , value(value_)
        , accessCount(1)
        , prev(nullptr)
        , next(nullptr)
    {}

    Key getKey() const { return key; }
    Value getValue() const { return value; }
    void setValue(const Value& value_) { value = value_; }
    size_t getAcceseeCount() const { return accessCount; }
    void increasementAccessCount() { ++ accessCount; }

    friend class LruCache<Key, Value>; //让 LruCache 能够访问 LruNode 类的私有成员
};

template<typename Key, typename Value>
class LruCache : public CachePolicy<Key, Value> {
public:
    using LruNodeType = LruNode<Key, Value>; // 定义 LruNode 类型别名
    using NodePtr = std::shared_ptr<LruNodeType>; // 定义 NodePtr 类型别名

    LruCache(size_t capacity_) : capacity(capacity_) {
        dummy = std::make_shared<LruNode<Key, Value>>(Key(), Value());
        dummy->prev = dummy;
        dummy->next = dummy;
    }
    ~LruCache() override = default;

    bool get(Key key, Value& value) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (key_to_node.find(key) == key_to_node.end()) {
            return false;
        }
        // 如果 key 存在，先通过哈希表定位，再移到头部
        auto node = key_to_node[key];
        moveToHead(node);
        value = node->value;
        return true;
    }
    void put(Key key, Value value) override { // 将值 value 关联到 key 上
        std::lock_guard<std::mutex> lock(mtx); //加锁
        if (key_to_node.find(key) != key_to_node.end()) { //如果 key 已经存在
            auto node = key_to_node[key];
            node->setValue(value);
            moveToHead(node);
        } else { //如果 key 不存在
            auto node = std::make_shared<LruNodeType>(key, value);
            key_to_node[key] = node;
            addToHead(node);
            if (key_to_node.size() > capacity) {
                auto back_node = dummy->prev;
                removeNode(back_node);
                key_to_node.erase(back_node->key); // 从哈希表中删除
            }
        }
    }

    Value get(Key key) override {
        Value value;
        if (!get(key, value)) {
            // throw std::runtime_error("key not found");
            return -1;
        }
        return value;
    }

    void removeNode(NodePtr node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    
    void moveToHead(NodePtr node) {
        removeNode(node);
        addToHead(node);
    }
    
    void addToHead(NodePtr node) {
        node->next = dummy->next;
        node->prev = dummy;
        dummy->next->prev = node;
        dummy->next = node;
    }
private:
    size_t capacity; //缓存容量
    std::unordered_map<Key, std::shared_ptr<LruNodeType>> key_to_node;
    std::shared_ptr<LruNodeType> dummy;
    std::mutex mtx;
};

}