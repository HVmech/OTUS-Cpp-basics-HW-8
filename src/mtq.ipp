template <typename T>
void mtq<T>::push(T value)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_queue.push(value);
    m_cv.notify_one();
}

template <typename T>
bool mtq<T>::pop(T& value)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this] () {
        return !m_queue.empty() || m_stopped;
    });

    bool finish = m_queue.empty() && m_stopped;

    if (!m_queue.empty()) {
        value = m_queue.front();
        m_queue.pop();
    }
    return !finish;
}

template <typename T>
void mtq<T>::stop()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_stopped = true;
    m_cv.notify_all();
}