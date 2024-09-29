#include <gs/util/string.h>
#include <gs/util/test.h>

int main(int argc, char** argv)
{
    PGS_ARENA Arena = GsArena();

    PGS_STRING Empty = GsStringInit(Arena);
    GS_REQUIRE(Empty != NULL);
    GS_REQUIRE(Empty->Capacity == 0);
    GS_REQUIRE(Empty->Length == 0);

    PGS_STRING WithContent = GsStringInitWithContent(Arena, "Hello, World!");
    GS_REQUIRE(WithContent != NULL);
    GS_REQUIRE(WithContent->Capacity > 0);
    GS_REQUIRE(WithContent->Length == strlen("Hello, World!"));
    GS_REQUIRE(strncmp(WithContent->Content, "Hello, World!", WithContent->Capacity) == 0);
    GS_REQUIRE(GsStringFindFirstOf(WithContent, 0, "H") == 0);
    GS_REQUIRE(GsStringFindFirstOf(WithContent, 0, "e") == 1);
    GS_REQUIRE(GsStringFindFirstOf(WithContent, 0, "o") == 4);
    GS_REQUIRE(GsStringFindFirstOf(WithContent, 0, "x") == SIZE_MAX);

    GS_REQUIRE(GsStringFindLastOf(WithContent, 0, "H") == 0);
    GS_REQUIRE(GsStringFindLastOf(WithContent, 0, "o") == 8);
    GS_REQUIRE(GsStringFindLastOf(WithContent, 5, "o") == 4);
    GS_REQUIRE(GsStringFindLastOf(WithContent, 0, "x") == SIZE_MAX);

    PGS_STRING WideCharString = GsStringInitWithWideContent(Arena, L"Hello, World!");
    GS_REQUIRE(WideCharString != NULL);
    GS_REQUIRE(strcmp(WideCharString->Content, "Hello, World!") == 0);

    Empty = GsStringInit(Arena);
    GS_REQUIRE(Empty != NULL);
    GS_REQUIRE(Empty->Capacity == 0);
    GS_REQUIRE(Empty->Length == 0);

    GS_REQUIRE(GsStringConcat(Empty, "Hello, World!") == GsStringSuccess);
    GS_REQUIRE(Empty->Length == strlen("Hello, World!"));
    GS_REQUIRE(strncmp(Empty->Content, "Hello, World!", Empty->Capacity) == 0);

    GsArenaRelease(Arena);
}
