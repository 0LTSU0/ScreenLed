#pragma once

#include <chrono>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <fstream>
#include <limits>
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
    int val,
    bool clock_as_val = false)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Frame dimensions must be positive");
    if (val < 0 || val > 255)
        throw std::invalid_argument("Val needs to be 8bit (0-255)");

    RawPixelBuffer frame;
    frame.width = width;
    frame.height = height;
    frame.rgb.assign(width * height * 3, val);
    
    if (clock_as_val) {
        frame.ss_timestamp = std::chrono::steady_clock::time_point{
            std::chrono::seconds{ val }
        };
    }
    else {
        frame.ss_timestamp = std::chrono::steady_clock::now();
    }
    
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

struct frameInstruction {
    int start_x = 0;
    int start_y = 0;
    int end_x = 0;
    int end_y = 0;
    rgbValue rgb{};

    bool isValid() const
    {
        return start_x >= 0 && start_x < end_x && start_y >= 0 && start_y < end_y;
    }
};

inline int getRgbVectorPos(int x, int y, int width)
{
    return (y * width + x) * 3;
}

inline RawPixelBuffer createCustomFrame(int width, int height, std::vector<frameInstruction> instructions)
{
    auto frame = RawPixelBuffer();
    frame.height = height;
    frame.width = width;
    frame.rgb.assign(width * height * 3, 0);

    for (const auto& inst : instructions)
    {
        if (!inst.isValid())
        {
            continue;
        }

        for (int x = inst.start_x; x < inst.end_x; x++)
        {
            for (int y = inst.start_y; y < inst.end_y; y++)
            {
                frame.rgb[getRgbVectorPos(x, y, width)] = inst.rgb.r;
                frame.rgb[getRgbVectorPos(x, y, width) + 1] = inst.rgb.g;
                frame.rgb[getRgbVectorPos(x, y, width) + 2] = inst.rgb.b;
            }
        }
    }

    return frame;
}

inline void saveRawPixelBufferAsImage(const RawPixelBuffer& frame, const char* filename)
{
    if (frame.width <= 0 || frame.height <= 0)
        throw std::invalid_argument("Invalid image dimensions");

    const std::uint64_t rowBytes = std::uint64_t(frame.width) * 3;
    const std::uint64_t stride = (rowBytes + 3) & ~std::uint64_t(3);
    const std::uint64_t dataSize = stride * frame.height;

    if (dataSize > UINT32_MAX - 54 ||
        std::uint64_t(frame.rgb.size()) != rowBytes * frame.height)
        throw std::invalid_argument("Invalid or oversized RGB buffer");

    std::ofstream out;
    out.exceptions(std::ios::failbit | std::ios::badbit);
    out.open(filename, std::ios::binary | std::ios::trunc);

    // BMP stores integers in little-endian order.
    auto write16 = [&](std::uint16_t value) {
        out.put(static_cast<char>(value & 0xff));
        out.put(static_cast<char>((value >> 8) & 0xff));
        };
    auto write32 = [&](std::uint32_t value) {
        write16(static_cast<std::uint16_t>(value));
        write16(static_cast<std::uint16_t>(value >> 16));
        };

    // File header (14 bytes).
    out.write("BM", 2);
    write32(static_cast<std::uint32_t>(54 + dataSize));
    write16(0);
    write16(0);
    write32(54);

    // Bitmap information header (40 bytes).
    write32(40);
    write32(static_cast<std::uint32_t>(frame.width));
    write32(static_cast<std::uint32_t>(frame.height));
    write16(1);   // Color planes.
    write16(24);  // Bits per pixel.
    write32(0);   // No compression.
    write32(static_cast<std::uint32_t>(dataSize));
    write32(0);
    write32(0);
    write32(0);
    write32(0);

    // BMP uses bottom-up rows, BGR channels, and 4-byte row alignment.
    const char padding[3] = {};
    for (int y = frame.height - 1; y >= 0; --y) {
        const auto* row = frame.rgb.data() +
            static_cast<std::size_t>(y) * rowBytes;

        for (int x = 0; x < frame.width; ++x) {
            const auto* pixel = row + static_cast<std::size_t>(x) * 3;
            out.put(static_cast<char>(pixel[2]));
            out.put(static_cast<char>(pixel[1]));
            out.put(static_cast<char>(pixel[0]));
        }

        out.write(padding, static_cast<std::streamsize>(stride - rowBytes));
    }

    out.close();
}