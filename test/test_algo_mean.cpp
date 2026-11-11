#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>

#include <Mailbox.h>
#include <Commons.h>
#include <helpers.h>

#include <algomean.h>

class AlgoTestMean : public ::testing::Test {
protected:
	
	std::vector<rgbValue> m_res;
	ScreenCapConfig m_conf;
	AlgoMean m_algo;

	const int testFrameWidth = 1920;
	const int testFrameHeight = 1080;

	void SetUp() override
	{
		m_algo = AlgoMean();
		m_conf = ScreenCapConfig();
		m_conf.c_algo = ScreenLedAlgorithm::MEAN_DEFAULT;
		m_res.assign(NUM_LED_SEGMENTS, rgbValue{});
	}

	void TearDown() override
	{
	}

};

TEST_F(AlgoTestMean, test_fullArea_solidFrames)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	m_algo.analyzeColors(m_res, m_conf, makeConsistentFrame(1920, 1080, 100));
	for (const auto &resSeg : m_res)
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

TEST_F(AlgoTestMean, test_fullArea_multiColor1)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;

	// Center third of the frame is red
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, testFrameHeight / 3, testFrameWidth, (testFrameHeight / 3) * 2, { 255,0,0 } });
	
	m_algo.analyzeColors(m_res, m_conf, createCustomFrame(testFrameWidth, testFrameHeight, inst));
	ASSERT_EQ(m_res.size(), NUM_LED_SEGMENTS);
	for (const auto& resSeg : m_res)
	{
		EXPECT_EQ(resSeg.r, 85); // third of the segment is red -> should end up 255/3=85
		EXPECT_EQ(resSeg.g, 0);
		EXPECT_EQ(resSeg.b, 0);
	}
}

TEST_F(AlgoTestMean, test_fullArea_multiColor2)
{
	m_conf.c_analyzerScreenArea = activeScreenArea::FULL;
	int segment_width = testFrameWidth / NUM_LED_SEGMENTS;

	// Fully green frame with blue square rectangle in the middle
	std::vector<frameInstruction> inst;
	inst.push_back(frameInstruction{ 0, 0, testFrameWidth, testFrameHeight,{ 0,0,255 } });
	inst.push_back(frameInstruction{ segment_width * 8, testFrameHeight / 3, segment_width * 12, (testFrameHeight / 3) * 2, { 0,255,0 } });

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
		EXPECT_EQ(m_res.at(i).g, 85);
		EXPECT_EQ(m_res.at(i).b, 170);
	}
	for (int i = 12; i < 20; i++)
	{
		EXPECT_EQ(m_res.at(i).r, 0);
		EXPECT_EQ(m_res.at(i).g, 0);
		EXPECT_EQ(m_res.at(i).b, 255);
	}
}

TEST_F(AlgoTestMean, test_fullArea_multiColor3)
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
			expected = rgbValue{ 85, 0, 170 };     // Red + blue + blue.
		else if (i >= 6 && i < 10)
			expected = rgbValue{ 85, 85, 85 };     // Red + green + blue.
		else if (i >= 10 && i < 14)
			expected = rgbValue{ 85, 170, 85 };    // Blue + green + yellow.
		else if (i >= 14 && i < 18)
			expected = rgbValue{ 85, 85, 170 };    // Blue + blue + yellow.

		EXPECT_EQ(m_res.at(i).r, expected.r);
		EXPECT_EQ(m_res.at(i).g, expected.g);
		EXPECT_EQ(m_res.at(i).b, expected.b);
	}
}
