#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <stdbool.h>
#include "packet_interface.h"

struct __attribute__((__packed__)) pkt
{
    ptypes_t type;
    uint16_t length;
    uint8_t TR;
    uint8_t window;
    uint8_t L;
    uint8_t seqnum;
    uint32_t timestamp;
    uint32_t crc1;
    char payload[512];
    uint32_t crc2;
};

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

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

void test_pkt_tr(void)
{
    pkt_t *pkt = pkt_new();
    pkt_set_tr(pkt, 0);
    CU_ASSERT_EQUAL(pkt_get_tr(pkt), 0);
    pkt_set_tr(pkt, 1);
    CU_ASSERT_EQUAL(pkt_get_tr(pkt), 1);
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
    CU_ASSERT_EQUAL(1,retval);
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
        NULL == CU_add_test(pSuite1, "\n\n……… Testing_tr……..\n\n", test_pkt_tr))
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
    if (NULL == CU_add_test(pSuite2, "\n\n………testing_vaurint_encode_and_decode……..\n\n", test_varuint_encode_decode))
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