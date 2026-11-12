#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>

#include <Mailbox.h>
#include <Commons.h>
#include <helpers.h>

#include <algomedian.h>

class AlgoTestMedian : public ::testing::Test {
protected:

	std::vector<rgbValue> m_res;
	ScreenCapConfig m_conf;
	AlgoMedian m_algo;

	const int testFrameWidth = 1920;
	const int testFrameHeight = 1080;

	void SetUp() override
	{
		m_algo = AlgoMedian();
		m_conf = ScreenCapConfig();
		m_conf.c_algo = ScreenLedAlgorithm::MEDIAN;
		m_res.assign(NUM_LED_SEGMENTS, rgbValue{});
	}

	void TearDown() override
	{
	}

};

TEST_F(AlgoTestMedian, test_fullArea_solidFrames)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	m_algo.analyzeColors(m_res, m_conf, makeConsistentFrame(1920, 1080, 100));
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 100);
		EXPECT_EQ(resSeg.g, 100);
		EXPECT_EQ(resSeg.b, 100);
	}

	m_algo.analyzeColors(m_res, m_conf, makeConsistentFrame(1920, 1080, 255));
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 255);
		EXPECT_EQ(resSeg.g, 255);
		EXPECT_EQ(resSeg.b, 255);
	}
}

TEST_F(AlgoTestMedian, test_fullArea_multiColor1)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	// Center third of the frame is red
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, testFrameHeight / 3, testFrameWidth, (testFrameHeight / 3) * 2,{ 255,0,0 } });

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 0);
		EXPECT_EQ(resSeg.g, 0);
		EXPECT_EQ(resSeg.b, 0);
	}
}

TEST_F(AlgoTestMedian, test_fullArea_multiColor2)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;
	int segment_width = testFrameWidth / NUM_LED_SEGMENTS;

	// Fully green frame with blue square rectangle in the middle
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, 0, testFrameWidth, testFrameHeight,{ 0,0,255 } });
	inst.push_back(frameInstruction{ segment_width * 8, 0, segment_width * 12, (testFrameHeight / 3) * 2,{ 0,255,0 } });

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);

	// segments 0..7 should be Green, 8..11 should be (0,85,170), 12..19 should be Green
	for (int i = 0; i < 8; i++)
	{
		EXPECT_EQ(m_res.at(i).r, 0);
		EXPECT_EQ(m_res.at(i).g, 0);
		EXPECT_EQ(m_res.at(i).b, 255);
	}
	for (int i = 8; i < 12; i++)
	{
		EXPECT_EQ(m_res.at(i).r, 0);
		EXPECT_EQ(m_res.at(i).g, 255);
		EXPECT_EQ(m_res.at(i).b, 0);
	}
	for (int i = 12; i < 20; i++)
	{
		EXPECT_EQ(m_res.at(i).r, 0);
		EXPECT_EQ(m_res.at(i).g, 0);
		EXPECT_EQ(m_res.at(i).b, 255);
	}
}

TEST_F(AlgoTestMedian, test_fullArea_multiColor3)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	ASSERT_EQ(NUM_LED_SEGMENTS, 20);
	ASSERT_EQ(testFrameWidth % NUM_LED_SEGMENTS, 0);
	ASSERT_EQ(testFrameHeight % 3, 0);

	const int sw = testFrameWidth / NUM_LED_SEGMENTS;
	const int third = testFrameHeight / 3;

	std::vector<frameInstruction> inst{
		// Blue background.
		{ 0, 0, testFrameWidth, testFrameHeight,{ 0, 0, 255 } },

		// Red top third in segments 2..9.
		{ sw * 2, 0, sw * 10, third,{ 255, 0, 0 } },

		// Green middle third in segments 6..13.
		{ sw * 6, third, sw * 14, third * 2,{ 0, 255, 0 } },

		// Yellow bottom third in segments 10..17.
		{ sw * 10, third * 2, sw * 18, testFrameHeight,{ 255, 255, 0 } }
	};

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);

	for (int i = 0; i < NUM_LED_SEGMENTS; ++i)
	{
		SCOPED_TRACE(i);

		rgbValue expected{ 0, 0, 255 };

		if (i >= 2 && i < 6)
			expected = rgbValue{ 0, 0, 255 };     // Blue dominates
		else if (i >= 6 && i < 10)
			expected = rgbValue{ 0, 0, 0 };     // Equal parts rgb -> mean ends up black because 0s dominate
		else if (i >= 10 && i < 14)
			expected = rgbValue{ 0, 255, 0 };    // Blue + green + yellow -> green component dominates
		else if (i >= 14 && i < 18)
			expected = rgbValue{ 0, 0, 255 };    // Blue dominates

		EXPECT_EQ(m_res.at(i).r, expected.r);
		EXPECT_EQ(m_res.at(i).g, expected.g);
		EXPECT_EQ(m_res.at(i).b, expected.b);
	}
}


TEST_F(AlgoTestMedian, test_fullArea_multiColor4)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	ASSERT_EQ(NUM_LED_SEGMENTS, 20);
	ASSERT_EQ(testFrameWidth % NUM_LED_SEGMENTS, 0);
	ASSERT_EQ(testFrameHeight % 3, 0);

	const int sw = testFrameWidth / NUM_LED_SEGMENTS;
	const int third = testFrameHeight / 3;

	std::vector<frameInstruction> inst{
		// Blueish background.
		{ 0, 0, testFrameWidth, testFrameHeight,{ 0, 173, 255 } },

		// Redish top third in segments 2..9.
		{ sw * 2, 0, sw * 10, third,{ 255, 55, 0 } },

		// Greenish middle third in segments 6..13.
		{ sw * 6, third, sw * 14, third * 2,{ 0, 255, 31 } },

		// Yellowish bottom third in segments 10..17.
		{ sw * 10, third * 2, sw * 18, testFrameHeight,{ 248, 255, 0 } }
	};

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);

	for (int i = 0; i < NUM_LED_SEGMENTS; ++i)
	{
		SCOPED_TRACE(i);

		rgbValue expected{ 0, 173, 255 }; // Blue dominates

		if (i >= 6 && i < 10)
			expected = rgbValue{ 0, 173, 31 };     // Equal parts rgb -> mean ends up black because 0s dominate
		else if (i >= 10 && i < 14)
			expected = rgbValue{ 0, 255, 31 };    // Blue + green + yellow -> green component dominates

		EXPECT_EQ(m_res.at(i).r, expected.r);
		EXPECT_EQ(m_res.at(i).g, expected.g);
		EXPECT_EQ(m_res.at(i).b, expected.b);
	}
}

TEST_F(AlgoTestMedian, test_centerThird_solidFrame)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::CENTER_THIRD;

	m_algo.analyzeColors(m_res, m_conf, makeConsistentFrame(1920, 1080, 255));
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 255);
		EXPECT_EQ(resSeg.g, 255);
		EXPECT_EQ(resSeg.b, 255);
	}
}

TEST_F(AlgoTestMedian, test_centerThird_multiColor1)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::CENTER_THIRD;

	// Center third of the frame is blue
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, testFrameHeight / 3, testFrameWidth, (testFrameHeight / 3) * 2,{ 0,0,255 } });

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 0);
		EXPECT_EQ(resSeg.g, 0);
		EXPECT_EQ(resSeg.b, 255); // third of the segment is blue -> should end up 255. if full frame was analyzed then median would be black
	}
}

TEST_F(AlgoTestMedian, test_centerThird_multiColor2)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::CENTER_THIRD;

	// Blue frame with thin red stripe
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, 0, testFrameWidth, testFrameHeight,{ 0,0,255 } });
	inst.push_back(frameInstruction{ 0, 400, testFrameWidth, 460,{ 255,0,0 } });

	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 0);
		EXPECT_EQ(resSeg.g, 0);
		EXPECT_EQ(resSeg.b, 255); // center third of the segment is blue -> should end up 255
	}
}

TEST_F(AlgoTestMedian, test_centerThird_multiColor3)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::CENTER_THIRD;

	// Red background, center third is blue, smaller green area in the middle
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, 0, testFrameWidth, testFrameHeight,{ 255,0,0 } });
	inst.push_back(frameInstruction{ 0, testFrameHeight / 3, testFrameWidth, (testFrameHeight / 3) * 2,{ 0,0,255 } });
	inst.push_back(frameInstruction{ testFrameWidth / 3, (testFrameHeight / 6) * 3, (testFrameWidth / 3) * 2, (testFrameHeight / 8) * 5,{ 0,255,0 } });

	saveRawPixelBufferAsImage(createCustomFrame(testFrameWidth, testFrameHeight, inst), "C:\\Users\\lauri\\Desktop\\ScreenLed_master\\ScreenLed\\test\\asd3.bmp");
	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);

	// Green is small enough that median ends up blue everywhere
	for (int i = 0; i < NUM_LED_SEGMENTS; i++)
	{
		EXPECT_EQ(m_res[i].r, 0);
		EXPECT_EQ(m_res[i].g, 0);
		EXPECT_EQ(m_res[i].b, 255);
	}
}
