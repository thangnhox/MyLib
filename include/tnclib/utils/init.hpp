#pragma once

namespace tnclib::utils {

    /**
     * Initializes default platform-specific implementations for utils subsystems.
     *
     * This function registers the built-in "system" backends for components such
     * as EventDispatcher and Network. It must be called before creating or using
     * these subsystems unless custom backends are registered manually.
     *
     * Safe to call multiple times.
     */
    void init_default_implementation();
}
