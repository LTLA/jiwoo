#include <gtest/gtest.h>

#include "jiwoo/EquilengthArrays.hpp"

#include <numeric>
#include <string>

TEST(EquilengthArrays, DefaultConstructor) {
    jiwoo::EquilengthArrays<double> basic;
    EXPECT_EQ(basic.size(), 0);
    EXPECT_EQ(basic.length(), 0);
    EXPECT_TRUE(basic.get() == NULL);
    EXPECT_TRUE(basic.empty());
}

TEST(EquilengthArrays, NonFillConstructor) {
    jiwoo::EquilengthArrays<double> basic(15, 20);
    EXPECT_EQ(basic.size(), 15);
    EXPECT_EQ(basic.length(), 20);
    EXPECT_TRUE(basic.get() != NULL);
    EXPECT_FALSE(basic.empty());
}

TEST(EquilengthArrays, Methods) {
    jiwoo::EquilengthArrays<double> basic(15, 20);

    EXPECT_TRUE(basic[0] != NULL);
    EXPECT_TRUE(basic[14] != NULL);

    // We can set things.
    basic[0][0] = 1;
    basic[14][19] = 100;
    EXPECT_EQ(basic[0][0], 1);
    EXPECT_EQ(basic[14][19], 100);

    basic.front()[1] = 2;
    basic.back()[18] = 200;
    EXPECT_EQ(basic.front()[1], 2);
    EXPECT_EQ(basic.back()[18], 200);

    // Loop iteration works as expected.
    for (auto ptr : basic) {
        ptr[5] = 5;
    }
    for (auto ptr : basic) {
        EXPECT_EQ(ptr[5], 5);
    }

    // Constant overloads work as well.
    const auto& conbasic = basic;
    EXPECT_TRUE(conbasic.get() != NULL);
    EXPECT_TRUE(conbasic[0] != NULL);
    EXPECT_TRUE(conbasic[14] != NULL);

    EXPECT_EQ(conbasic.front()[0], 1);
    EXPECT_EQ(conbasic.back()[19], 100);

    EXPECT_EQ(conbasic[0][1], 2);
    EXPECT_EQ(conbasic[14][18], 200);

    for (auto ptr : conbasic) {
        EXPECT_EQ(ptr[5], 5);
    }
}

TEST(EquilengthArrays, FillingConstructor) {
    jiwoo::EquilengthArrays<double> basic(5, 9, 99);
    EXPECT_EQ(basic.size(), 5);
    EXPECT_EQ(basic.length(), 9);
    EXPECT_EQ(basic[0][0], 99);
    EXPECT_EQ(basic[4][8], 99);
}

TEST(EquilengthArrays, MoveConstructor) {
    jiwoo::EquilengthArrays<double> basic(5, 9, 99);
    jiwoo::EquilengthArrays<double> eater(std::move(basic));
    EXPECT_EQ(eater.size(), 5);
    EXPECT_EQ(eater.length(), 9);
    EXPECT_EQ(eater[0][0], 99);
    EXPECT_EQ(eater[4][8], 99);
    EXPECT_EQ(basic.size(), 0);
}

TEST(EquilengthArrays, CopyAssignment) {
    jiwoo::EquilengthArrays<double> ref(11, 5);
    for (int i = 0; i < 11; ++i) {
        std::iota(ref[i], ref[i] + 5, i);
    }

    // No allocation required.
    {
        jiwoo::EquilengthArrays<double> src(11, 5);
        src = ref;
        for (int i = 0; i < 11; ++i) {
            for (int j = 0; j < 5; ++j) {
                EXPECT_EQ(src[i][j], i + j);
            }
        }
    }

    // Self allocation has no effect.
    // Note that a little song and dance is required to prevent compilers from detecting a self-assignment
    // (and generating a warning).
    {
        const auto addr = &ref;
        auto masked = reinterpret_cast<void*>(addr);
        ref = *reinterpret_cast<jiwoo::EquilengthArrays<double>*>(masked);
        for (int i = 0; i < 11; ++i) {
            for (int j = 0; j < 5; ++j) {
                EXPECT_EQ(ref[i][j], i + j);
            }
        }
    }

    // Allocating to something with its own existing memory.
    {
        jiwoo::EquilengthArrays<double> src(9, 20, 101);
        src = ref;
        EXPECT_EQ(src.size(), 11);
        EXPECT_EQ(src.length(), 5);
        for (int i = 0; i < 11; ++i) {
            for (int j = 0; j < 5; ++j) {
                EXPECT_EQ(src[i][j], i + j);
            }
        }
    }
}

TEST(EquilengthArrays, CopyAssignmentThrowable) {
    // Getting some coverage on the situation where the Value_'s copy constructor might throw.
    // This should cause the short-circuiting to skip.
    jiwoo::EquilengthArrays<std::string> ref(3, 4);

    {
        jiwoo::EquilengthArrays<std::string> src(3, 4);
        src = ref;
        EXPECT_EQ(src.size(), 3);
        EXPECT_EQ(src.length(), 4);
    }

    {
        jiwoo::EquilengthArrays<std::string> src(5, 6);
        src = ref;
        EXPECT_EQ(src.size(), 3);
        EXPECT_EQ(src.length(), 4);
    }
}

TEST(EquilengthArrays, MoveAssignment) {
    jiwoo::EquilengthArrays<double> ref(11, 5);
    for (int i = 0; i < 11; ++i) {
        std::iota(ref[i], ref[i] + 5, i);
    }

    jiwoo::EquilengthArrays<double> src = std::move(ref);
    EXPECT_EQ(ref.size(), 0);
    EXPECT_EQ(src.size(), 11);
    EXPECT_EQ(src.length(), 5);

    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 5; ++j) {
            EXPECT_EQ(src[i][j], i + j);
        }
    }
}
