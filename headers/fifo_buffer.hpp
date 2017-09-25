#pragma once
#ifndef FIFO_BUFFER_HPP
#define FIFO_BUFFER_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <map>

template <class T>
class FiFoBuffer {
public:
    FiFoBuffer(size_t _maxSize = 20) : maxBufferSize(0), itemCounter(0), maxSize(_maxSize){}
    bool Push(const T &item) {
        std::lock_guard<std::mutex> lock(p_queue_mutex);
        size_t size = p_buffer.size();
        if (maxSize > 0 && size > maxSize)
            return false;
        p_buffer.push(item);
        return true;
    }
    bool Pop(T &item) {
        std::lock_guard<std::mutex> lock(p_queue_mutex);
        if (p_buffer.empty())
            return false;
        item = p_buffer.front();
        p_buffer.pop();
        return true;
    }
    size_t Size(){
        std::lock_guard<std::mutex> lock(p_queue_mutex);
        return p_buffer.size();
    }
    ~FiFoBuffer() {
        while (!p_buffer.empty()) {
            p_buffer.pop();
        }
    }
private:
    std::queue<T> p_buffer;
    std::mutex p_queue_mutex;
    size_t maxBufferSize, maxSize;
    size_t itemCounter;
    std::map<void*, int> dataMemoryCounter;
    std::map<void*, int> matrixMemoryCounter;
};

#endif // FIFO_BUFFER_HPP
