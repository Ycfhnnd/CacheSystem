#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "CachePolicy.h"

namespace Cache{

template<typename Key, typename Value> class LfuCache;

template<typename Key, typename Value>
class LfuNode{
private:
    Key key;
    Value value;
    size_t freq;
    std::shared_ptr<LfuNode<Key, Value>> prev;
    std::shared_ptr<LfuNode<Key, Value>> next;
public:
    LfuNode(Key key_, Value value_)
        : key(key_)
        , value(value_)
        , freq(1)
        , prev(nullptr)
        , next(nullptr)
    {}

    Key getKey() const { return key; }
    Value getValue() const { return value; }
    void setValue(const Value& value_) { value = value_; }
    size_t getAcceseeCount() const { return freq; }
    void increasementAccessCount() { ++ freq; }
    std::shared_ptr<LfuNode<Key, Value>> getPrev() const { return prev; }
    std::shared_ptr<LfuNode<Key, Value>> getNext() const { return next; }
    void setPrev(std::shared_ptr<LfuNode<Key, Value>> prev_) { prev = prev_; }
    void setNext(std::shared_ptr<LfuNode<Key, Value>> next_) { next = next_; }

    friend class LfuCache<Key, Value>; //让 LruCache 能够访问 LruNode 类的私有成员
};

template<typename Key, typename Value>
class LfuCache : public CachePolicy<Key, Value>{
public:
    using LfuNodeType = LfuNode<Key, Value>;
    using NodePtr = std::shared_ptr<LfuNodeType>;

    LfuCache(size_t capacity_)
        : capacity(capacity_)
        , minFreq(0)
    {}
    ~LfuCache() override = default;

    bool get(Key key, Value& value) override {
        std::lock_guard<std::mutex>lock(mtx);

        auto it = key_to_node.find(key);
        if (it == key_to_node.end()) {
            return false;
        }
        NodePtr node = it->second;
        removeNode(node);
        NodePtr dummy = freq_to_dummy[node->getAcceseeCount()];
        if (dummy->getPrev() == dummy) {
            freq_to_dummy.erase(node->getAcceseeCount());
            if (minFreq == node->getAcceseeCount()) {
                ++minFreq;
            }
        }
        node->increasementAccessCount();
        moveToHead(node->getAcceseeCount(), node);
        value = node->getValue();
        return true;
    }   

    void put(Key key, Value value) override {
        std::lock_guard<std::mutex>lock(mtx); 

        auto it = key_to_node.find(key);
        if (it != key_to_node.end()) {
            NodePtr node = it->second;
            node->setValue(value);
            get(key, value);
            return;
        }
        if (key_to_node.size() == capacity) {
            NodePtr dummy = freq_to_dummy[minFreq];
            NodePtr node = dummy->getPrev();
            removeNode(node);
            key_to_node.erase(node->getKey());
            if (dummy->getPrev() == dummy) {
                freq_to_dummy.erase(minFreq);
            }
        }
        NodePtr node = std::make_shared<LfuNodeType>(key, value);
        key_to_node[key] = node;
        moveToHead(1, node);
        minFreq = 1;
    }

    Value get(Key key) override {
        Value value;
        if (!get(key, value)) {
            return -1;
        }
        return value;
    }

private:
    void removeNode(NodePtr node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void moveToHead(size_t freq, NodePtr x) {
        auto it = freq_to_dummy.find(freq);
        if (it == freq_to_dummy.end()) {
            it = freq_to_dummy.emplace(freq, new_list()).first;
        }
        NodePtr dummy = it->second;
        x->next = dummy->next;
        x->prev = dummy;
        dummy->next->prev = x;
        dummy->next = x;
    }
    
    NodePtr new_list() {
        NodePtr dummy = std::make_shared<LfuNodeType>(Key(), Value());
        dummy->setPrev(dummy);
        dummy->setNext(dummy);
        return dummy;
    }
private:
    std::mutex mtx;
    std::unordered_map<Key, NodePtr> key_to_node;
    std::unordered_map<Key, NodePtr> freq_to_dummy;
    size_t capacity;
    size_t minFreq;
};

}