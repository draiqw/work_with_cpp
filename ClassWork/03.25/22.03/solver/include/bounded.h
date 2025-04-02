#include <deque>
#include <mutex>
#include <condition_variable>

template <typename T>
class Bounded {
    std::deque<T> buffer;
    size_t capacity;
    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

public:
    Bounded(size_t capacity) : capacity(capacity) {}

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        not_full.wait(lock, [this]() {
            return buffer.size() < capacity;
        });

        buffer.push_front(std::move(item));
        not_empty.notify_one();
    }

    void pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        not_empty.wait(lock, [this]() {
            return !buffer.empty();
        });

        item = std::move(buffer.back());
        buffer.pop_back();
        not_full.notify_one();
    }
};
