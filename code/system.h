//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "common.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/commander.h"

class System {
    
public:
    Renderer*  m_renderer  = nullptr;
    
    Renderer *  getRenderer () { return m_renderer; }
    Commander*  getCommander() { return m_renderer->getCommander(); }
    Swapchain*  getSwapchain() { return m_renderer->getSwapchain(); }
    
    static System& instance() {
        static System instance; // Guaranteed to be destroyed. Instantiated on first use.
        return instance;
    }
    
private:
    
    System() {}
    ~System() {}

    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are unacceptable otherwise you may accidentally get copies of
    // your singleton appearing.
    System(System const&);         // Don't Implement
    void operator=(System const&); // Don't implement
    
};

