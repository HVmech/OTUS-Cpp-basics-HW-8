// Template class mtq
// Thread-safe queue based on std::queue

#include <queue>
#include <thread>
#include <mutex>

template <typename T>
class mtq
{
public:
    void push(T value);

    bool pop(T& value);

    void stop();

private:
    std::condition_variable m_cv;
    std::queue<T> m_queue;
    std::mutex m_mtx;
    bool m_stopped = false;
};

#include "mtq.ipp"