#include <chrono>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>

inline RawPixelBuffer makeRandomFrame(
    int width,
    int height,
    unsigned int seed = std::random_device{}())
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Frame dimensions must be positive");

    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);

    if (w > std::numeric_limits<std::size_t>::max() / h / 3)
        throw std::length_error("Frame dimensions are too large");

    RawPixelBuffer frame;
    frame.width = width;
    frame.height = height;
    frame.rgb.resize(w * h * 3);

    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> channel(0, 255);

    for (auto& value : frame.rgb)
        value = static_cast<unsigned char>(channel(generator));

    frame.ss_timestamp = std::chrono::steady_clock::now();
    return frame;
}

inline RawPixelBuffer makeConsistentFrame(
    int width,
    int height,
    int val)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Frame dimensions must be positive");
    if (val < 0 || val > 255)
        throw std::invalid_argument("Val needs to be 8bit (0-255)");

    RawPixelBuffer frame;
    frame.width = width;
    frame.height = height;
    frame.rgb.assign(width * height * 3, val);
    frame.ss_timestamp = std::chrono::steady_clock::time_point{
        std::chrono::seconds{ val }
    };

    return frame;
}

inline bool compareRawPixelBuffers(RawPixelBuffer buf1, RawPixelBuffer buf2)
{
    if (buf1.height != buf2.height ||
        buf1.width != buf2.width ||
        buf1.ss_timestamp != buf2.ss_timestamp ||
        buf1.rgb.size() != buf2.rgb.size())
    {
        return false;
    }

    for (int i = 0; i < buf1.rgb.size(); i++)
    {
        if (buf1.rgb.at(i) != buf2.rgb.at(i))
        {
            return false;
        }
    }

    return true;
}