#include "timed.h"

Timed::Timed(unsigned long delayms)
  : m_timer(0),
    m_delay(delayms),
    m_active(true),
    m_stopwatch()
{
    m_stopwatch.start();
    m_timer = m_stopwatch.milliseconds();
}

void Timed::setDelay(unsigned long delayms)
{
    unsigned long now = m_stopwatch.milliseconds();
    m_delay = delayms;
    m_timer = now+m_delay;
}

bool Timed::update()
{
    return update(m_stopwatch.milliseconds());
}

bool Timed::update(unsigned long now)
{
    if((long)(now-m_timer)>=0 && m_active)
    {
        m_timer = now+m_delay;
        fire(now);
        return true;
    }
    return false;
}

void Timed::stop()
{
    m_active=false;
}

void Timed::start()
{
    m_active=true;
}

void Timed::reset()
{
    
    m_timer = m_stopwatch.milliseconds()+m_delay;
}


void Timed::fire(unsigned long now)
{
    //do nothing
}

long Timed::mins(unsigned long now)
{
    return now/1000/60;
}

long Timed::secs(unsigned long now)
{
    long mins=now/1000/60;
    return now/1000-mins*60;
}

long Timed::tenth(unsigned long now)
{
    long mins=now/1000/60;
    long secs=now/1000-mins*60;
    return (now-(mins*60+secs)*1000)/100;
}

