#include <gs/util/wstring.h>
#include <gs/util/string.h>
#include <gs/util/test.h>

int main(int argc, char** argv)
{
    PGS_ARENA Arena = GsArena();

    PGS_WSTRING Empty = GsWStringInit(Arena);
    GS_REQUIRE(Empty != NULL);
    GS_REQUIRE(Empty->Capacity == 0);
    GS_REQUIRE(Empty->Length == 0);

    PGS_WSTRING WithContent = GsWStringInitWithContent(Arena, L"Hello, World!");
    GS_REQUIRE(WithContent != NULL);
    GS_REQUIRE(WithContent->Capacity > 0);
    GS_REQUIRE(WithContent->Length == wcslen(L"Hello, World!"));
    GS_REQUIRE(wcsncmp(WithContent->Content, L"Hello, World!", WithContent->Capacity) == 0);
    GS_REQUIRE(GsWStringFindFirstOf(WithContent, 0, L"H") == 0);
    GS_REQUIRE(GsWStringFindFirstOf(WithContent, 0, L"e") == 1);
    GS_REQUIRE(GsWStringFindFirstOf(WithContent, 0, L"o") == 4);
    GS_REQUIRE(GsWStringFindFirstOf(WithContent, 0, L"x") == SIZE_MAX);

    GS_REQUIRE(GsWStringFindLastOf(WithContent, 0, L"H") == 0);
    GS_REQUIRE(GsWStringFindLastOf(WithContent, 0, L"o") == 8);
    GS_REQUIRE(GsWStringFindLastOf(WithContent, 5, L"o") == 4);
    GS_REQUIRE(GsWStringFindLastOf(WithContent, 0, L"x") == SIZE_MAX);

    PGS_STRING NarrowCharString = GsStringInitWithContent(Arena, "Hello, World!");
    GS_REQUIRE(NarrowCharString != NULL);

    PGS_WSTRING ConvertedString = GsWStringInitWithString(NarrowCharString);
    GS_REQUIRE(ConvertedString != NULL);

    GS_REQUIRE(wcsncmp(ConvertedString->Content, L"Hello, World!", WithContent->Capacity) == 0);

    Empty = GsWStringInit(Arena);
    GS_REQUIRE(Empty != NULL);
    GS_REQUIRE(Empty->Capacity == 0);
    GS_REQUIRE(Empty->Length == 0);

    GS_REQUIRE(GsWStringConcat(Empty, L"Hello, World!") == GsWStringSuccess);
    GS_REQUIRE(Empty->Length == wcslen(L"Hello, World!"));
    GS_REQUIRE(wcsncmp(Empty->Content, L"Hello, World!", Empty->Capacity) == 0);

    GsArenaRelease(Arena);
}
