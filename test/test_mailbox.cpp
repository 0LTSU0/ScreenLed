#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>

#include <Mailbox.h>
#include <Commons.h>
#include <helpers.h>

class MailboxTest : public ::testing::Test {
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}

};

TEST_F(MailboxTest, test_put_take)
{
	auto mailbox = LatestOnlyMailbox<int>();

	mailbox.put(1);
	EXPECT_EQ(mailbox.get(), 1);
	mailbox.put(2);
	EXPECT_EQ(mailbox.get(), 2);
}

TEST_F(MailboxTest, test_latest_only)
{
	auto mailbox = LatestOnlyMailbox<int>();

	mailbox.put(1);
	mailbox.put(2);
	EXPECT_EQ(mailbox.get(), 2);

	mailbox.put(3);
	mailbox.put(4);
	EXPECT_EQ(mailbox.get(), 4);
}

TEST_F(MailboxTest, test_latest_only_RawPixelBuffer)
{
	auto mailbox = LatestOnlyMailbox<RawPixelBuffer>();
	auto rawFrame = makeRandomFrame(1920, 1080);
	mailbox.put(rawFrame);
    EXPECT_TRUE(compareRawPixelBuffers(mailbox.get(), rawFrame));

	auto rawFrame2 = makeRandomFrame(1920, 1080);
	auto rawFrame3 = makeRandomFrame(1920, 1080);
	mailbox.put(rawFrame2);
	mailbox.put(rawFrame3);
    EXPECT_TRUE(compareRawPixelBuffers(mailbox.get(), rawFrame3));
}

TEST_F(MailboxTest, concurrent_producer_consumer)
{
    LatestOnlyMailbox<RawPixelBuffer> mailbox;

    std::atomic<bool> corrupted{ false };
    std::atomic<int> received{ 0 };
    static int frameCount = 255;

    std::thread consumer([&]() {
        while (true) {
            RawPixelBuffer frame = mailbox.get();
            
            //std::cout << "Consumed frame" << std::endl;

            if (frame.width == 1 && frame.height == 1) { break; }
            if (frame.width * frame.height * 3 != frame.rgb.size())
            {
                std::cout << "Received frame size and rgb vector size do not match" << std::endl;
                corrupted = true;
            }

            int valExpected = std::chrono::duration_cast<std::chrono::seconds>(
                frame.ss_timestamp.time_since_epoch()
            ).count();
            for (const auto &val : frame.rgb) {
                if (val != valExpected) {
                    corrupted = true;
                    std::cout << "Received frame data is corrupt. Different value found from rgb array than what frame timestamp is" << std::endl;
                }
            }

            ++received;
        }
    });

    std::thread producer([&]() {
        for (int i = 0; i < frameCount; ++i) {
            auto frame = makeConsistentFrame(123, 1234, i, true);
            mailbox.put(frame);
            //std::cout << "Produced frame" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // roughly "60 FPS"
        }

        // Tell consumer to exit.
        mailbox.put(makeConsistentFrame(1, 1, 1, true));
    });

    producer.join();
    consumer.join();

    EXPECT_FALSE(corrupted.load());

    EXPECT_GT(received.load(), 0);
    EXPECT_LE(received.load(), frameCount);
}
