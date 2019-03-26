#ifndef NULS_CONSTANTS_H
#define NULS_CONSTANTS_H

#define NEED_NEXT_CHUNK 0x6866

#define MAX_BIP32_PATH 10
#define MAX_BIP32_PATH_LENGTH (4 * MAX_BIP32_PATH) + 1

#define MAX_OUTPUT_TO_CHECK 10

#define TX_TYPE_1_CONSENSUS_REWARD 1
#define TX_TYPE_2_TRANSFER_TX 2
#define TX_TYPE_3_SET_ALIAS 3
#define TX_TYPE_4_REGISTER_CONSENSUS_NODE 4
#define TX_TYPE_5_JOIN_CONSENSUS 5
#define TX_TYPE_6_CANCEL_CONSENSUS 6
#define TX_TYPE_7_YELLOW_CARD 7
#define TX_TYPE_8_RED_CARD 8
#define TX_TYPE_9_UNREGISTER_CONSENSUS_NODE 9
#define TX_TYPE_10_BUSINESS_DATA 10
#define TX_TYPE_100_CREATE_CONTRACT 100
#define TX_TYPE_101_CALL_CONTRACT 101
#define TX_TYPE_102_DELETE_CONTRACT 102
#define TX_TYPE_103_TRANSFER_CONTRACT 103

#define ADDRESS_TYPE_P2PKH 1
#define ADDRESS_TYPE_CONTRACT 2
#define ADDRESS_TYPE_P2SH 3

#define HASH_LENGTH 34
#define ADDRESS_LENGTH 23 // chainid (2) + addressType (1) + RIPEMID160 (20)
#define BASE58_ADDRESS_LENGTH 32

#define MAX_REMARK_LENGTH 30
#define MAX_ALIAS_LENGTH 20
#define AMOUNT_LENGTH 8
#define LOCKTIME_LENGTH 6
#define DIGEST_LENGTH 32

#define MAX_METHODNAME_LENGTH 150

//Hash of address Nse5FeeiYk1opxdc5RqYpEWkiUDGNuLs
extern const unsigned char BLACK_HOLE_ADDRESS[ADDRESS_LENGTH];
extern const unsigned char BLACK_HOLE_ALIAS_AMOUNT[AMOUNT_LENGTH];

extern const unsigned char MIN_DEPOSIT_REGISTER_AGENT[AMOUNT_LENGTH];
extern const unsigned char MIN_DEPOSIT_JOIN_CONSENSUS[AMOUNT_LENGTH];


#endif