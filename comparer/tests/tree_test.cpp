#include <iostream>
#include <cassert>
#include "../headers/tree.h"

void testBuildTree() {
    Tree tree("../tests/test_ast.txt");
    Node* root = tree.getRoot();

    // Verify the root node
    assert(root != nullptr);
    assert(root->type == "Declaration");
    assert(root->kind == "TranslationUnit");
    assert(root->usr == "c:");
    assert(root->path == "N/A");
    assert(root->lineNumber == 0);
    assert(root->columnNumber == 0);

    // Verify the root's children
    assert(root->children.size() == 16);

    // Verify the first child node
    Node* child1 = root->children[0];
    assert(child1->type == "Declaration");
    assert(child1->kind == "Typedef");
    assert(child1->usr == "c:@T@_Float32");
    assert(child1->path == "/usr/include/x86_64-linux-gnu/bits/floatn-common.h");
    assert(child1->lineNumber == 214);
    assert(child1->columnNumber == 1);

    // Verify the second child node
    Node* child2 = root->children[1];
    assert(child2->type == "Declaration");
    assert(child2->kind == "Typedef");
    assert(child2->usr == "c:@T@_Float64");
    assert(child2->path == "/usr/include/x86_64-linux-gnu/bits/floatn-common.h");
    assert(child2->lineNumber == 251);
    assert(child2->columnNumber == 1);

    // Verify the third child node
    Node* child3 = root->children[2];
    assert(child3->type == "Declaration");
    assert(child3->kind == "Typedef");
    assert(child3->usr == "c:@T@_Float32x");
    assert(child3->path == "/usr/include/x86_64-linux-gnu/bits/floatn-common.h");
    assert(child3->lineNumber == 268);
    assert(child3->columnNumber == 1);

    // Verify the fourth child node
    Node* child4 = root->children[3];
    assert(child4->type == "Declaration");
    assert(child4->kind == "Typedef");
    assert(child4->usr == "c:@T@_Float64x");
    assert(child4->path == "/usr/include/x86_64-linux-gnu/bits/floatn-common.h");
    assert(child4->lineNumber == 285);
    assert(child4->columnNumber == 1);

    // Verify the fifth child node
    Node* child5 = root->children[4];
    assert(child5->type == "Declaration");
    assert(child5->kind == "Typedef");
    assert(child5->usr == "c:@T@size_t");
    assert(child5->path == "/usr/lib/clang/14/include/stddef.h");
    assert(child5->lineNumber == 46);
    assert(child5->columnNumber == 1);

    // Verify the sixth child node
    Node* child6 = root->children[5];
    assert(child6->type == "Declaration");
    assert(child6->kind == "Typedef");
    assert(child6->usr == "c:@T@va_list");
    assert(child6->path == "/usr/lib/clang/14/include/stdarg.h");
    assert(child6->lineNumber == 14);
    assert(child6->columnNumber == 1);

    // Verify the seventh child node
    Node* child7 = root->children[6];
    assert(child7->type == "Declaration");
    assert(child7->kind == "Typedef");
    assert(child7->usr == "c:@T@__gnuc_va_list");
    assert(child7->path == "/usr/lib/clang/14/include/stdarg.h");
    assert(child7->lineNumber == 32);
    assert(child7->columnNumber == 1);

    // Verify the eighth child node
    Node* child8 = root->children[7];
    assert(child8->type == "Declaration");
    assert(child8->kind == "Typedef");
    assert(child8->usr == "c:@T@wint_t");
    assert(child8->path == "/usr/include/x86_64-linux-gnu/bits/types/wint_t.h");
    assert(child8->lineNumber == 20);
    assert(child8->columnNumber == 1);

    // Verify the ninth child node
    Node* child9 = root->children[8];
    assert(child9->type == "Declaration");
    assert(child9->kind == "CXXRecord");
    assert(child9->usr == "c:@SA@__mbstate_t");
    assert(child9->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9->lineNumber == 13);
    assert(child9->columnNumber == 9);

    // Verify the ninth child's children
    assert(child9->children.size() == 3);

    // Verify the first child of the ninth child node
    Node* child9_1 = child9->children[0];
    assert(child9_1->type == "Declaration");
    assert(child9_1->kind == "Field");
    assert(child9_1->usr == "c:@SA@__mbstate_t@FI@__count");
    assert(child9_1->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9_1->lineNumber == 15);
    assert(child9_1->columnNumber == 3);

    // Verify the second child of the ninth child node
    Node* child9_2 = child9->children[1];
    assert(child9_2->type == "Declaration");
    assert(child9_2->kind == "CXXRecord");
    assert(child9_2->usr == "c:@SA@__mbstate_t@U@__mbstate_t.h@451");
    assert(child9_2->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9_2->lineNumber == 16);
    assert(child9_2->columnNumber == 3);

    // Verify the children of the second child of the ninth child node
    assert(child9_2->children.size() == 2);

    // Verify the first child of the second child of the ninth child node
    Node* child9_2_1 = child9_2->children[0];
    assert(child9_2_1->type == "Declaration");
    assert(child9_2_1->kind == "Field");
    assert(child9_2_1->usr == "c:@SA@__mbstate_t@U@__mbstate_t.h@451@FI@__wch");
    assert(child9_2_1->path == "N/A");
    assert(child9_2_1->lineNumber == 110);
    assert(child9_2_1->columnNumber == 23);

    // Verify the second child of the second child of the ninth child node
    Node* child9_2_2 = child9_2->children[1];
    assert(child9_2_2->type == "Declaration");
    assert(child9_2_2->kind == "Field");
    assert(child9_2_2->usr == "c:@SA@__mbstate_t@U@__mbstate_t.h@451@FI@__wchb");
    assert(child9_2_2->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9_2_2->lineNumber == 19);
    assert(child9_2_2->columnNumber == 5);

    // Verify the child of the second child of the second child of the ninth child node
    assert(child9_2_2->children.size() == 1);
    Node* child9_2_2_1 = child9_2_2->children[0];
    assert(child9_2_2_1->type == "Statement");
    assert(child9_2_2_1->kind == "IntegerLiteral");
    assert(child9_2_2_1->usr == "N/A");
    assert(child9_2_2_1->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9_2_2_1->lineNumber == 19);
    assert(child9_2_2_1->columnNumber == 17);

    // Verify the third child of the ninth child node
    Node* child9_3 = child9->children[2];
    assert(child9_3->type == "Declaration");
    assert(child9_3->kind == "Field");
    assert(child9_3->usr == "c:@SA@__mbstate_t@FI@__value");
    assert(child9_3->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child9_3->lineNumber == 16);
    assert(child9_3->columnNumber == 3);

    // Verify the tenth child node
    Node* child10 = root->children[9];
    assert(child10->type == "Declaration");
    assert(child10->kind == "Typedef");
    assert(child10->usr == "c:@T@__mbstate_t");
    assert(child10->path == "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h");
    assert(child10->lineNumber == 13);
    assert(child10->columnNumber == 1);

    // Verify the eleventh child node
    Node* child11 = root->children[10];
    assert(child11->type == "Declaration");
    assert(child11->kind == "Typedef");
    assert(child11->usr == "c:@T@mbstate_t");
    assert(child11->path == "/usr/include/x86_64-linux-gnu/bits/types/mbstate_t.h");
    assert(child11->lineNumber == 6);
    assert(child11->columnNumber == 1);

    // Verify the twelfth child node
    Node* child12 = root->children[11];
    assert(child12->type == "Declaration");
    assert(child12->kind == "CXXRecord");
    assert(child12->usr == "c:@S@_IO_FILE");
    assert(child12->path == "/usr/include/x86_64-linux-gnu/bits/types/__FILE.h");
    assert(child12->lineNumber == 4);
    assert(child12->columnNumber == 1);

    // Verify the thirteenth child node
    Node* child13 = root->children[12];
    assert(child13->type == "Declaration");
    assert(child13->kind == "Typedef");
    assert(child13->usr == "c:@T@__FILE");
    assert(child13->path == "/usr/include/x86_64-linux-gnu/bits/types/__FILE.h");
    assert(child13->lineNumber == 5);
    assert(child13->columnNumber == 1);

    // Verify the fourteenth child node
    Node* child14 = root->children[13];
    assert(child14->type == "Declaration");
    assert(child14->kind == "CXXRecord");
    assert(child14->usr == "c:@S@_IO_FILE");
    assert(child14->path == "/usr/include/x86_64-linux-gnu/bits/types/FILE.h");
    assert(child14->lineNumber == 4);
    assert(child14->columnNumber == 1);

    // Verify the fifteenth child node
    Node* child15 = root->children[14];
    assert(child15->type == "Declaration");
    assert(child15->kind == "Typedef");
    assert(child15->usr == "c:@T@FILE");
    assert(child15->path == "/usr/include/x86_64-linux-gnu/bits/types/FILE.h");
    assert(child15->lineNumber == 7);
    assert(child15->columnNumber == 1);

    // Verify the sixteenth child node
    Node* child16 = root->children[15];
    assert(child16->type == "Declaration");
    assert(child16->kind == "CXXRecord");
    assert(child16->usr == "c:@S@__locale_struct");
    assert(child16->path == "/usr/include/x86_64-linux-gnu/bits/types/__locale_t.h");
    assert(child16->lineNumber == 27);
    assert(child16->columnNumber == 1);

    std::cout << "All tests passed!" << std::endl;
}

int main() {
    testBuildTree();
    return 0;
}