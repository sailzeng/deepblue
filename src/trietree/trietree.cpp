// trietree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file



#include "pch.h"
#include "zce_shm_trie_tree.h"
#include "interactive_movie_save.h"


int test_tie_tree()
{
    typedef ZCE_LIB::smem_trie_tree<int32_t>  Trie_Tree_TEST;

    const int32_t NUM_NODE = 10240;
    size_t sz_alloc = Trie_Tree_TEST::alloc_size(10240);
    std::cout << "Size alloc :" << sz_alloc << std::endl;
    char *shm_ptr = new char[sz_alloc];
    Trie_Tree_TEST *trie_tree = Trie_Tree_TEST::initialize(NUM_NODE, shm_ptr);

    std::cout << "Hello World!\n";

    const int32_t TEST_ARRAY1_LEN = 16;
    const int32_t TEST_ARRAY1[TEST_ARRAY1_LEN] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

    bool bret = false;
    size_t matching_len = 0;

    bret = trie_tree->insert_word(TEST_ARRAY1, TEST_ARRAY1_LEN);
    std::cout << "insert word 1 " << std::boolalpha << bret << std::endl;
    std::cout << "Size:" << trie_tree->size() << " Free:" << trie_tree->free() << std::endl;

    trie_tree->dump_tree();

    bret = trie_tree->has_word(TEST_ARRAY1, TEST_ARRAY1_LEN, &matching_len);
    std::cout << "find word 1 " << std::boolalpha << bret << " matching len:" << matching_len << std::endl;

    const int32_t TEST_ARRAY21_LEN = 16;
    const int32_t TEST_ARRAY21[TEST_ARRAY1_LEN] = { 66,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    bret = trie_tree->insert_word(TEST_ARRAY21, TEST_ARRAY21_LEN);
    std::cout << "insert word 21 " << std::boolalpha << bret << std::endl;
    std::cout << "Size:" << trie_tree->size() << " Free:" << trie_tree->free() << std::endl;

    const int32_t TEST_ARRAY2_LEN = 18;
    const int32_t TEST_ARRAY2[TEST_ARRAY2_LEN] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18 };

    bret = trie_tree->has_word(TEST_ARRAY2, TEST_ARRAY2_LEN, &matching_len);
    std::cout << "find word 2 " << std::boolalpha << bret << " matching len:" << matching_len << std::endl;

    bret = trie_tree->insert_word(TEST_ARRAY2, TEST_ARRAY2_LEN);
    std::cout << "insert word 2" << std::boolalpha << bret << std::endl;
    std::cout << "Size:" << trie_tree->size() << " Free:" << trie_tree->free() << std::endl;

    trie_tree->dump_tree();

    const int32_t TEST_ARRAY3_LEN = 18;
    const int32_t TEST_ARRAY3[TEST_ARRAY3_LEN] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,20,21 };

    bret = trie_tree->has_word(TEST_ARRAY3, TEST_ARRAY3_LEN, &matching_len);
    std::cout << "find word 3 " << std::boolalpha << bret << " matching len:" << matching_len << std::endl;

    bret = trie_tree->insert_word(TEST_ARRAY3, TEST_ARRAY3_LEN);

    std::cout << "insert word 3" << std::boolalpha << bret << std::endl;
    std::cout << "Size:" << trie_tree->size() << " Free:" << trie_tree->free() << std::endl;

    trie_tree->dump_tree();

    size_t letter_num;
    bret = trie_tree->has_letter(9, &letter_num);
    std::cout << "find letter 9 " << std::boolalpha << bret << " letter len:" << letter_num << std::endl;

    return 0;
}

int test_IM_Save()
{
    size_t last_save_array_num = 32;
    uint32_t recall_node[] = {1,2,3,10,11,12,20,32};
    size_t max_save_node_path_len = 1024;
    size_t recall_node_num = 8;
    Interactive_Movie_Save::config(last_save_array_num,
                                   max_save_node_path_len,
                                   recall_node,
                                   recall_node_num);

    Interactive_Movie_Save im_save;
    im_save.initialize();

    return 0;
}


int main()
{
    test_IM_Save();
    return 0;
}

