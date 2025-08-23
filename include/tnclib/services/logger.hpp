#pragma once

#include <string>
#include <string_view>

namespace tnclib {
    namespace services {

        class Logger {
        public:
            enum class Level { Trace = 0, Debug, Info, Warn, Error, Critical, Off }; // log level

            enum class Color : uint32_t {
                Black       = 0x000000FF,
                White       = 0xFFFFFFFF,
                Red         = 0xFF0000FF,
                Green       = 0x00FF00FF,
                Blue        = 0x0000FFFF,
                Yellow      = 0xFFFF00FF,
                Magenta     = 0xFF00FFFF,
                Cyan        = 0x00FFFFFF,
                LightGray   = 0xD3D3D3FF,
                DarkGray    = 0xA9A9A9FF,
                Orange      = 0xFFA500FF,
                Purple      = 0x800080FF,
                Brown       = 0xA52A2AFF,
                Pink        = 0xFFC0CBFF
            };



            void set_level(Level level);
            Level level() const;

            // color is set in rgba
            void set_header_color(Level level, uint32_t color); // set color of header
            void set_text_color(Level level, uint32_t color); // set color of text

            void set_header_color(Level level, Color color); // set color of header using preset color
            void set_text_color(Level level, Color color); // set color of text using preset color

            void set_default_colors(); // set color back to default

            void enable_colors(bool enable);

            void log(Level level, std::string_view message);

        public:
            static Logger& getInstance();

        private:
            Logger();
            ~Logger();


        private:
            struct Impl;
            Impl* pImpl;
        }; // logger

    } // namespace services
} // namespace tnclib