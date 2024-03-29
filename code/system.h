//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "common.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/commander.h"
#include "window/settings.h"

class System {
    
public:
    Renderer* m_pRenderer = nullptr;
    Settings* m_pSettings = nullptr;
    
    static Renderer * Renderer () { return Instance().m_pRenderer; }
    static Commander* Commander() { return Instance().m_pRenderer->getCommander(); }
    static Settings * Settings () { return Instance().m_pSettings; }
    
    static System& Instance() {
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

