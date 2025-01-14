#include "appslistmodel.h"

#define private public
#include "fullscreenframe.h"
#include "multipagesview.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Fullscreenframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_fullScreenFrame = new FullScreenFrame(nullptr);
    }

    void TearDown() override
    {
        delete m_fullScreenFrame;
        m_fullScreenFrame = nullptr;
    }

public:
    FullScreenFrame *m_fullScreenFrame;
};
