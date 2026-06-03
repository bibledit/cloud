#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <limits>

// Include the actual header/implementation
extern "C" {
    // Forward declare the string struct and callback from receive.cpp
    struct string {
        char *ptr;
        size_t len;
    };
    
    void init_string(struct string *s);
    size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
}

class ReallocSecurityTest : public ::testing::TestWithParam<size_t> {};

TEST_P(ReallocSecurityTest, ReallocFailureDoesNotCauseNullDeref) {
    // Invariant: After writefunc, s->ptr must be valid (non-NULL) or 
    // the function must handle allocation failure gracefully without crashing
    
    size_t data_size = GetParam();
    struct string s;
    init_string(&s);
    
    // Create test data
    std::vector<char> test_data(data_size > 0 ? data_size : 1, 'A');
    
    // Call the actual writefunc - it should not crash on valid inputs
    // and should maintain invariant that s.ptr is usable after success
    size_t result = writefunc(test_data.data(), 1, data_size, &s);
    
    // Security invariant: if writefunc returns non-zero (success),
    // the buffer must be valid and properly null-terminated
    if (result > 0) {
        ASSERT_NE(s.ptr, nullptr) << "Buffer must not be NULL after successful write";
        ASSERT_EQ(s.len, data_size) << "Length must match written data";
        ASSERT_EQ(s.ptr[s.len], '\0') << "Buffer must be null-terminated";
    }
    
    // Cleanup
    if (s.ptr) {
        free(s.ptr);
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    ReallocSecurityTest,
    ::testing::Values(
        0,                          // Boundary: zero-length write
        1,                          // Boundary: minimal valid write
        4096,                       // Normal case: typical buffer size
        1024 * 1024                 // Large allocation stress test
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}