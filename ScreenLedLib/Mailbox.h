#ifndef MAILBOX_H
#define MAILBOX_H

#include <QMutex>
#include <QWaitCondition>
#include <optional>

template<typename T>
class LatestOnlyMailbox {
public:
    void put(T item) {
        QMutexLocker lock(&m_mutex);
        m_item = std::move(item);   // overwrite
        m_hasItem = true;
        m_cond.wakeOne();
    }

    // Blocks until an item is available
    T get() {
        QMutexLocker lock(&m_mutex);
        while (!m_hasItem)
            m_cond.wait(&m_mutex);
        T item = std::move(*m_item);
        m_hasItem = false;
        return item;
    }

private:
    QMutex m_mutex;
    QWaitCondition m_cond;
    std::optional<T> m_item;
    bool m_hasItem = false;
};

#endif // MAILBOX_H
