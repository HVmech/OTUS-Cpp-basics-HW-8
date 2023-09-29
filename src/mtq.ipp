template <typename T>
void mtq<T>::push(T value)
{
    // Lock std::mutex with std::unique_lock
    std::unique_lock<std::mutex> lock(m_mtx);

    // Add value
    m_queue.push(value);

    // Notify one thread
    m_cv.notify_one();
}

template <typename T>
bool mtq<T>::pop(T& value)
{
    // Lock std::mutex with std::unique_lock
    std::unique_lock<std::mutex> lock(m_mtx);

    // Check std::conditional_variable
    m_cv.wait(lock, [this] () {
        return !m_queue.empty() || m_stopped;
    });

    // Flag
    bool finish = m_queue.empty() && m_stopped;

    // Get value from queue and call std::queue::pop()
    if (!m_queue.empty()) {
        value = m_queue.front();
        m_queue.pop();
    }

    // Return flag
    return !finish;
}

template <typename T>
void mtq<T>::stop()
{
    // Lock std::mutex with std::unique_lock
    std::unique_lock<std::mutex> lock(m_mtx);

    // Flag
    m_stopped = true;

    // Notify all threads
    m_cv.notify_all();
}
