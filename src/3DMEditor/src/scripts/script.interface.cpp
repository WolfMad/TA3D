/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "script.interface.h"
#include "../logs.h"

namespace TA3D
{
    ScriptInterface::ScriptInterface() : caller(NULL), running(false), waiting(false), sleeping(false), sleep_time(0.0f), signal_mask(0)
    {
    }

    void ScriptInterface::kill()
    {
        running = false;
    }

    void ScriptInterface::stop()
    {
        waiting = true;
    }

    void ScriptInterface::resume()
    {
        waiting = false;
    }

    void ScriptInterface::pause(float time)
    {
        sleeping = true;
        sleep_time = time;
    }

    void ScriptInterface::addThread(ScriptInterface *pChild)
    {
        if (caller == pChild) return;
        if (caller)
            caller->addThread(pChild);
        else
        {
            if (pChild == this) return;
            removeThread(pChild);
            childs.push_back(pChild);
        }
    }

    void ScriptInterface::removeThread(ScriptInterface *pChild)
    {
        if (caller == pChild) return;
        if (caller)
            caller->removeThread(pChild);
        else
            for(std::vector<ScriptInterface*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
                if (*i == pChild)
                {
                    childs.erase(i);
                    return;
                }
    }

    void ScriptInterface::processSignal(uint32 signal)
    {
        if (caller)
            caller->processSignal(signal);
        else
        {
            if (signal & getSignalMask())
                kill();
            for(std::vector<ScriptInterface*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
                if ((*i)->getSignalMask() & signal)
                    (*i)->kill();
        }
    }

    void ScriptInterface::setSignalMask(uint32 signal)
    {
        signal_mask = signal;
    }

    uint32 ScriptInterface::getSignalMask()
    {
        return signal_mask;
    }

    void ScriptInterface::deleteThreads()
    {
        for(int i = 0 ; i < childs.size() ; ++i)
            delete childs[i];
        childs.clear();
        for(std::deque<ScriptInterface*>::iterator i = freeThreads.begin() ; i != freeThreads.end() ; ++i)
            delete *i;
        freeThreads.clear();
    }

    ScriptInterface *ScriptInterface::getFreeThread()
    {
        if (caller)
            return caller->getFreeThread();

        if (freeThreads.empty())
            return NULL;
        ScriptInterface *newThread = freeThreads.front();
        freeThreads.pop_front();
        return newThread;
    }

    void ScriptInterface::clean()
    {
        if (caller)         // Don't go up to caller this would make complexity O(N²)!!
            return;         // and it would not be safe at all!
        else
        {
            int e = 0;
            for(int i = 0 ; i + e < childs.size() ; )
            {
                if (!childs[i + e]->is_self_running())
                {
                    freeThreads.push_back(childs[i + e]);      // Put it in the queue
                    ++e;
                }
                else
                {
                    childs[i] = childs[i + e];
                    ++i;
                }
            }
            if (e)
                childs.resize(childs.size() - e);
        }
    }

    void ScriptInterface::dumpDebugInfo()
    {
        LOG_DEBUG(LOG_PREFIX_SCRIPT << "sorry dumpDebugInfo not implemented for this type of script");
    }
}
