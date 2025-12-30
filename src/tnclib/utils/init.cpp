#include "tnclib/utils/init.hpp"

#include "tnclib/services/logger.hpp"

#include "tnclib/utils/event.hpp"
#include "tnclib/utils/network.hpp"

#include "platform/cross/event.hpp"
#include "platform/cross/network.hpp"

namespace tnclib::utils {

    void init_default_implementation() {
        LOG_INFO("Utils init: Adding default EventDispatcher implementation");
        tnclib::utils::EventDispatcher::RegisterBackend(
            "system",
            []() {
                return std::make_shared<tnclib::platform::SelectEventDispatcher>();
            }
        );

        LOG_INFO("Utils init: Adding default Network implementation");
        tnclib::utils::Network::RegisterBackend(
            "system",
            []() {
                return std::make_shared<tnclib::platform::CrossNetwork>();
            }
        );
    }

}
