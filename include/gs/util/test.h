#ifndef GS_TEST_H
#define GS_TEST_H

#include <stdio.h>

#define GS_REQUIRE_LINE(Condition, Line, ConditionString)                               \
if(!(Condition)) {                                                                      \
    printf("[FAIL] Condition %s on line %u not met.\n", ConditionString, Line);         \
    return -1;                                                                          \
}

#define GS_REQUIRE(Condition) GS_REQUIRE_LINE(Condition, __LINE__, #Condition)

#endif // GS_TEST_H
