#ifndef TIMED_H
#define TIMED_H

#include <stopwatch.h>

using namespace ssobjects;

class Timed
{
    protected:
        unsigned long m_timer;
        unsigned long m_delay;
        bool m_active;
        StopWatch m_stopwatch;

    public:
        Timed(unsigned long delayms);           ///< delay in milliseconds. Timer starts when constructed.
        bool hasFired();
        bool update();                          ///< Called when using internal stopwatch
        virtual bool update(unsigned long now); ///< tick with current time in milliseconds.
        virtual void fire(unsigned long now);   ///< called when the timer reaches it's end and has reset.
        void start();
        void stop();
        void reset();
        void setDelay(unsigned long delayms);
        //TODO handle > 59:59:9 - wrap back to 0
        long mins(unsigned long now);        ///< minutes              MM:ss.t
        long secs(unsigned long now);        ///< seconds              mm:SS.t
        long tenth(unsigned long now);       ///< tenth of a second    mm:ss.T
};

#endif

