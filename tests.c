#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <stdbool.h>
#include "packet_interface.h"
#include "sender_list.h"

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

void test_pkt_new(void)
{
    pkt_t *pkt = pkt_new();
    CU_ASSERT_PTR_NOT_NULL(pkt);
}

/* void test_pkt_decode(void){

}

void test_pkt_encode(void){
    
}
*/

void test_pkt_get_type(void)
{
    ptypes_t ptypes1 = PTYPE_ACK;
    ptypes_t ptypes2;
    pkt_t *pkt = pkt_new();
    pkt_set_type(pkt, ptypes1);
    ptypes2 = pkt_get_type(pkt);
    CU_ASSERT_EQUAL((int)ptypes2, (int)ptypes1);
}

void test_pkt_get_tr(void)
{
    pkt_t *pkt = pkt_new();
    pkt_set_tr(pkt, 1);
    CU_ASSERT_EQUAL(pkt, 1);
    pkt_set_tr(pkt, 0);
    CU_ASSERT_EQUAL(pkt, 0);
    pkt_set_tr(pkt, 37);
    CU_ASSERT_EQUAL(pkt, 153);
    pkt_set_tr(pkt, 153);
    CU_ASSERT_EQUAL(pkt, 153);
    pkt_set_tr(pkt, 235);
    CU_ASSERT_NOT_EQUAL(pkt, 234);
}

void test_pkt_get_window(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_set_window(pkt1, 5);
    pkt_t *pkt2 = pkt_new();
    pkt_set_window(pkt2, 31);
    pkt_t *pkt3 = pkt_new();
    pkt_set_window(pkt3, 26);
    CU_ASSERT_EQUAL(5, pkt_get_window(pkt1));
    CU_ASSERT_EQUAL(31, pkt_get_window(pkt2));
    CU_ASSERT_NOT_EQUAL(17, pkt_get_window(pkt3));
}

void test_pkt_get_lentgh(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_length(pkt1, 5555);
    pkt_set_length(pkt2, 3);
    pkt_set_length(pkt3, 15879);
    pkt_set_length(pkt4, 36);
    CU_ASSERT_EQUAL(5555, pkt_get_length(pkt1));
    CU_ASSERT_EQUAL(3, pkt_get_length(pkt2));
    CU_ASSERT_EQUAL(15879, pkt_get_length(pkt3));
    CU_ASSERT_NOT_EQUAL(23, pkt_get_length(pkt4));
}

void test_pkt_get_timestamp(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_timestamp(pkt1, 589999);
    pkt_set_timestamp(pkt2, 56);
    pkt_set_timestamp(pkt3, 589);
    pkt_set_timestamp(pkt4, 7894);
    CU_ASSERT_EQUAL(589999, pkt_get_timestamp(pkt1));
    CU_ASSERT_EQUAL(56, pkt_get_timestamp(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_timestamp(pkt3));
    CU_ASSERT_NOT_EQUAL(10, pkt_get_timestamp(pkt4));
}

void test_pkt_get_crc1(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_crc1(pkt1, 58999);
    pkt_set_crc1(pkt2, 27);
    pkt_set_crc1(pkt3, 589);
    pkt_set_crc1(pkt4, 458);
    CU_ASSERT_EQUAL(58999, pkt_get_crc1(pkt1));
    CU_ASSERT_EQUAL(27, pkt_get_crc1(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_crc1(pkt3));
    CU_ASSERT_NOT_EQUAL(459, pkt_get_crc1(pkt4));
}

void test_pkt_get_crc2(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_crc2(pkt1, 58999);
    pkt_set_crc2(pkt2, 27);
    pkt_set_crc2(pkt3, 589);
    pkt_set_crc2(pkt4, 458);
    CU_ASSERT_EQUAL(58999, pkt_get_crc2(pkt1));
    CU_ASSERT_EQUAL(27, pkt_get_crc2(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_crc2(pkt3));
    CU_ASSERT_NOT_EQUAL(459, pkt_get_crc2(pkt4));
}

/*void test_pkt_get_playload(void){
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_payload(pkt1,"coucou bae",58999);
    pkt_set_payload(pkt2,"coucou arnaud",588);
    pkt_set_payload(pkt3,"coucou patrick",37);
    pkt_set_payload(pkt4,"coucou nicki",17);
    CU_ASSERT_EQUAL("coucou bae",58999);
    CU_ASSERT_EQUAL("coucou arnaud",588);
    CU_ASSERT_EQUAL("coucou patrick",37);
    CU_ASSERT_NOT_EQUAL("coucou nicki",17);
} */

void test_pkt_set_type(void)
{
    ptypes_t ptypes1 = PTYPE_ACK;
    ptypes_t ptypes2;
    pkt_t *pkt = pkt_new();
    pkt_set_type(pkt, ptypes1);
    ptypes2 = pkt_get_type(pkt);
    CU_ASSERT_EQUAL((int)ptypes2, (int)ptypes1);
}


void test_pkt_set_tr(void)
{
    pkt_t *pkt = pkt_new();
    pkt_set_tr(pkt, 254);
    CU_ASSERT_EQUAL(pkt, 254);
    pkt_set_tr(pkt, 18);
    CU_ASSERT_EQUAL(pkt, 37);
    pkt_set_tr(pkt, 37);
    CU_ASSERT_EQUAL(pkt, 153);
    pkt_set_tr(pkt, 153);
    CU_ASSERT_EQUAL(pkt, 153);
    pkt_set_tr(pkt, 224);
    CU_ASSERT_NOT_EQUAL(pkt, 225);
}

void test_pkt_set_window(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_set_window(pkt1, 5);
    pkt_t *pkt2 = pkt_new();
    pkt_set_window(pkt2, 31);
    pkt_t *pkt3 = pkt_new();
    pkt_set_window(pkt3, 26);
    CU_ASSERT_EQUAL(5, pkt_get_window(pkt1));
    CU_ASSERT_EQUAL(31, pkt_get_window(pkt2));
    CU_ASSERT_NOT_EQUAL(17, pkt_get_window(pkt3));
}

void test_pkt_set_length(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_length(pkt1, 5555);
    pkt_set_length(pkt2, 3);
    pkt_set_length(pkt3, 15879);
    pkt_set_length(pkt4, 36);
    CU_ASSERT_EQUAL(5555, pkt_get_length(pkt1));
    CU_ASSERT_EQUAL(3, pkt_get_length(pkt2));
    CU_ASSERT_EQUAL(15879, pkt_get_length(pkt3));
    CU_ASSERT_NOT_EQUAL(23, pkt_get_length(pkt4));
}

void test_pkt_set_timestamp(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_timestamp(pkt1, 589999);
    pkt_set_timestamp(pkt2, 56);
    pkt_set_timestamp(pkt3, 589);
    pkt_set_timestamp(pkt4, 7894);
    CU_ASSERT_EQUAL(589999, pkt_get_timestamp(pkt1));
    CU_ASSERT_EQUAL(56, pkt_get_timestamp(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_timestamp(pkt3));
    CU_ASSERT_NOT_EQUAL(10, pkt_get_timestamp(pkt4));
}

void test_pkt_set_crc1(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_crc1(pkt1, 58999);
    pkt_set_crc1(pkt2, 27);
    pkt_set_crc1(pkt3, 589);
    pkt_set_crc1(pkt4, 458);
    CU_ASSERT_EQUAL(58999, pkt_get_crc1(pkt1));
    CU_ASSERT_EQUAL(27, pkt_get_crc1(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_crc1(pkt3));
    CU_ASSERT_NOT_EQUAL(459, pkt_get_crc1(pkt4));
}

void test_pkt_set_crc2(void)
{
    pkt_t *pkt1 = pkt_new();
    pkt_t *pkt2 = pkt_new();
    pkt_t *pkt3 = pkt_new();
    pkt_t *pkt4 = pkt_new();
    pkt_set_crc2(pkt1, 58999);
    pkt_set_crc2(pkt2, 27);
    pkt_set_crc2(pkt3, 589);
    pkt_set_crc2(pkt4, 458);
    CU_ASSERT_EQUAL(58999, pkt_get_crc2(pkt1));
    CU_ASSERT_EQUAL(27, pkt_get_crc2(pkt2));
    CU_ASSERT_EQUAL(589, pkt_get_crc2(pkt3));
    CU_ASSERT_NOT_EQUAL(459, pkt_get_crc2(pkt4));
}

/*void test_pkt_set_playload(void){
    
}
*/

void test_pkt_type(void)
{
    pkt_t *pkt = pkt_new();
    pkt_set_type(pkt, PTYPE_DATA);
    CU_ASSERT_EQUAL(pkt_get_type(pkt), PTYPE_DATA);
    pkt_set_type(pkt, PTYPE_NACK);
    CU_ASSERT_EQUAL(pkt_get_type(pkt), PTYPE_NACK);
    pkt_set_type(pkt, PTYPE_ACK);
    CU_ASSERT_EQUAL(pkt_get_type(pkt), PTYPE_ACK);
}


void test_varuint_encode_decode(void)
{
    uint8_t data[2];
    uint16_t val = 100;
    uint16_t retval;
    varuint_encode(val, data, 2);
    varuint_decode(data, 2, &retval);
    CU_ASSERT_EQUAL(val, retval);
    uint8_t tester = 0b00000001;
    varuint_decode(&tester, 1, &retval);
    CU_ASSERT_EQUAL(1, retval);
}

void test_add_list(void)
{

    list_t *my_list = init_list();
    pkt_t *pkt1 = pkt_new();
    pkt_set_payload(pkt1, "1", 2);
    pkt_t *pkt2 = pkt_new();
    pkt_set_payload(pkt2, "2", 2);
    pkt_t *pkt3 = pkt_new();
    pkt_set_payload(pkt3, "3", 2);
    pkt_t *pkt4 = pkt_new();
    pkt_set_payload(pkt4, "4", 2);
    pkt_t *pkt5 = pkt_new();
    pkt_set_payload(pkt5, "5", 2);

    pkt_t *printer;

    add(my_list, pkt1);
    printer = peek_last(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "1");
    add(my_list, pkt2);
    printer = peek_last(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "2");
    add(my_list, pkt3);
    printer = peek_last(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "3");
    add(my_list, pkt4);
    printer = peek_last(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "4");
    add(my_list, pkt5);
    printer = peek_last(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "5");
}

void test_delete_list(void)
{

    list_t *my_list = init_list();
    pkt_t *pkt1 = pkt_new();
    pkt_set_payload(pkt1, "1", 2);
    pkt_t *pkt2 = pkt_new();
    pkt_set_payload(pkt2, "2", 2);
    pkt_t *pkt3 = pkt_new();
    pkt_set_payload(pkt3, "3", 2);
    pkt_t *pkt4 = pkt_new();
    pkt_set_payload(pkt4, "4", 2);
    pkt_t *pkt5 = pkt_new();
    pkt_set_payload(pkt5, "5", 2);

    pkt_t *printer;

    add(my_list, pkt1);
    add(my_list, pkt2);
    add(my_list, pkt3);
    add(my_list, pkt4);
    add(my_list, pkt5);

    printer = peek(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "1");
    delete (my_list);
    printer = peek(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "2");
    delete (my_list);
    printer = peek(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "3");
    delete (my_list);
    printer = peek(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "4");
    delete (my_list);
    printer = peek(my_list);
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(printer), "5");
}

int main(void)
{
    CU_pSuite pSuite1, pSuite2 = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) // Initialize CUnit test registry
    {
        return CU_get_error();
    }

    pSuite1 = CU_add_suite("Test_on_getter_and_setter", init_suite, clean_suite); //creating test suit for getters and setters
    if (NULL == pSuite1)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite1, "\n\n……… Testing_type……..\n\n", test_pkt_type) ||
        NULL == CU_add_test(pSuite1, "\n\n………testing_vaurint_encode_and_decode……..\n\n", test_varuint_encode_decode) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_add_list……..\n\n", test_add_list) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_delete_list……..\n\n", test_delete_list) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_new_pkt……..\n\n", test_pkt_new) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_type……..\n\n", test_pkt_get_type) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_window……..\n\n", test_pkt_get_window) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_length……..\n\n", test_pkt_get_lentgh) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_timestamp……..\n\n", test_pkt_get_timestamp) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_crc1……..\n\n", test_pkt_get_crc1) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_crc2……..\n\n", test_pkt_get_crc2) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_window……..\n\n", test_pkt_set_window) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_length……..\n\n", test_pkt_set_length) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_timestamp……..\n\n", test_pkt_set_timestamp) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_crc1……..\n\n", test_pkt_set_crc1) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_crc2……..\n\n", test_pkt_set_crc2) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_get_tr……..\n\n", test_pkt_get_tr) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_set_tr……..\n\n", test_pkt_set_tr))

    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite2 = CU_add_suite("Test_on_varuint", init_suite, clean_suite); //creating suit for varuint
    if (NULL == pSuite2)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite1, "\n\n……… Testing_add_list……..\n\n", test_add_list) ||
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_delete_list……..\n\n", test_delete_list))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_NORMAL);

    CU_basic_run_tests(); // OUTPUT to the screen
    CU_basic_show_failures(CU_get_failure_list());

    CU_cleanup_registry(); //Cleaning the Registry
    return CU_get_error();
}