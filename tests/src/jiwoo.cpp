#include <gtest/gtest.h>

#include "jiwoo/jiwoo.hpp"

TEST(Scope, Vector) {
    {
        std::vector<double*> blah;
        jiwoo::Scope sc(blah);
    }

    {
        std::vector<double*> blah(10); // All null pointers.
        jiwoo::Scope sc(blah);
    }

    {
        std::vector<int*> blah(10);
        for (auto& x : blah) {
            x = new int[20];
        }
        jiwoo::Scope sc(blah);
    }

    {
        std::vector<std::vector<float*> > blah(5);
        for (auto& x : blah) {
            x.resize(10);
            for (auto& y : x) {
                y = new float[10];
            }
        }
        jiwoo::Scope sc(blah);
    }
}

TEST(Scope, Optional) {
    {
        std::optional<std::vector<double*> > blah;
        jiwoo::Scope sc(blah);
    }

    {
        std::optional<std::vector<double*> > blah;
        blah.emplace(20);
        for (auto& x : *blah) {
            x = new double[5];
        }
        jiwoo::Scope sc(blah);
    }

    {
        std::vector<std::optional<double*> > blah(13);
        jiwoo::Scope sc(blah);
    }

    {
        std::optional<std::vector<std::optional<std::vector<double*> > > > blah;
        blah.emplace(20);
        for (int i = 0; i < 20; ++i) {
            if (i % 2 == 0) {
                continue;
            }
            auto& x = (*blah)[i];
            x.emplace(5);
            for (int j = 0; j < 5; ++j) {
                if (j % 2 == 1) {
                    (*x)[j] = new double[j + 10];
                }
            }
        }
        jiwoo::Scope sc(blah);
    }
}

/*****************************************/

class TransferTest : public ::testing::TestWithParam<bool> {};

TEST_P(TransferTest, Vector) {
    auto force_copy = GetParam();

    {
        std::vector<double*> from, to;
        jiwoo::Scope scf(from), sct(to);

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_TRUE(from.empty());
        EXPECT_TRUE(to.empty());
    }

    {
        std::vector<double*> from;
        std::vector<double*> to;
        jiwoo::Scope scf(from), sct(to);
        from.resize(10);

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_TRUE(from.empty());
        EXPECT_EQ(to, std::vector<double*>(10));
    }

    {
        std::vector<int*> from, to;
        jiwoo::Scope scf(from), sct(to);
        from.resize(10);
        for (int i = 0; i < 10; ++i) {
            from[i] = new int[i + 10];
            from[i][0] = i;
        }

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_TRUE(from.empty());
        EXPECT_EQ(to.size(), 10);
        for (int i = 0; i < 10; ++i) {
            EXPECT_EQ(to[i][0], i);
        }
    }

    {
        std::vector<std::vector<double*> > from, to;
        jiwoo::Scope scf(from), sct(to);
        from.resize(10);
        for (int i = 0; i < 10; ++i) {
            from[i].resize(5);
            for (int j = 0; j < 5; ++j) {
                from[i][j] = new double [j + 1];
                from[i][j][0] = (i + 1) * (j + 1);
            }
        }

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_TRUE(from.empty());
        EXPECT_EQ(to.size(), 10);
        for (int i = 0; i < 10; ++i) {
            EXPECT_EQ(to[i].size(), 5);
            for (int j = 0; j < 5; ++j) {
                EXPECT_EQ(to[i][j][0], (i + 1) * (j + 1));
            }
        }
    }
}

TEST_P(TransferTest, Optional) {
    auto force_copy = GetParam();

    {
        std::optional<double*> from, to;
        jiwoo::Scope scf(from), sct(to);

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_FALSE(from.has_value());
        EXPECT_FALSE(to.has_value());
    }

    {
        std::optional<double*> from, to;
        jiwoo::Scope scf(from), sct(to);
        auto ptr = new double[12];
        from = ptr;

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_FALSE(from.has_value());
        EXPECT_EQ(*to, ptr);
    }

    {
        std::optional<std::vector<int*> > from, to;
        jiwoo::Scope scf(from), sct(to);
        from.emplace(12);
        for (int i = 0; i < 12; ++i) {
            (*from)[i] = new int[i + 1];
            (*from)[i][0] = i * 2;
        }

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_FALSE(from.has_value());
        EXPECT_EQ(to->size(), 12);
        for (int i = 0; i < 12; ++i) {
            EXPECT_EQ((*to)[i][0], i * 2);
        }
    }

    {
        std::optional<std::vector<std::optional<std::vector<double*> > > > from, to;
        jiwoo::Scope scf(from), sct(to);
        from.emplace(3);
        for (int i = 0; i < 3; ++i) {
            auto& x = (*from)[i];
            x.emplace(4);
            for (int j = 0; j < 4; ++j) {
                (*x)[j] = new double[5];
                (*x)[j][0] = 10 * i + j;
            }
        }

        if (force_copy) {
            jiwoo::transfer_internal<true>(from, to);
        } else {
            jiwoo::transfer(from, to);
        }

        EXPECT_FALSE(from.has_value());
        EXPECT_EQ(to->size(), 3);
        for (int i = 0; i < 3; ++i) {
            auto& x = (*to)[i];
            EXPECT_EQ(x->size(), 4);
            for (int j = 0; j < 4; ++j) {
                EXPECT_EQ((*x)[j][0], 10 * i + j);
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    Transfer,
    TransferTest,
    ::testing::Values(false, true)
);
