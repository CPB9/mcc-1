#pragma once

#include "mcc/Config.h"

#include <QObject>

#include <cstddef>

namespace mccui {

template <typename P>
class QObjectRefCountable : public P {
public:
    template <typename... A>
    QObjectRefCountable(A&&... args)
        : P(std::forward<A>(args)...)
        , _rc(0)
    {
    }

    virtual ~QObjectRefCountable()
    {
    }

protected:
    std::size_t numRefs() const
    {
        return _rc;
    }

private:
    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(QObjectRefCountable);

    friend void bmclRcAddRef(const QObjectRefCountable<P>* rc)
    {
        rc->_rc++;
    }

    friend void bmclRcRelease(const QObjectRefCountable<P>* rc)
    {
        rc->_rc--;
        if (rc->_rc == 0) {
            if (rc->parent() == nullptr) {
                const_cast<QObjectRefCountable<P>*>(rc)->deleteLater();
            }
        }
    }

    mutable std::size_t _rc;
};
}
