#include "common_parser.h"
#include "100_create_contract.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_100_create_contract_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create contract from", 0x01),
  TITLE_ITEM("Contract address", 0x02),
  TITLE_ITEM("Value", 0x03),
  TITLE_ITEM("Gas Limit", 0x04),
  TITLE_ITEM("Price", 0x05),
  TITLE_ITEM("Fees", 0x06),
  TITLE_ITEM("Code Hash", 0x07),
  TITLE_ITEM("Arguments", 0x08),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_ARROW_RIGHT(0x05),
  ICON_ARROW_RIGHT(0x06),
  ICON_ARROW_RIGHT(0x07),
  ICON_CHECK(0x08),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_100_create_contract(uint8_t step) {
  return step + 1;
}

static void uiProcessor_100_create_contract(uint8_t step) {
  unsigned short amountTextSize;
  tx_type_specific_100_create_contract_t *cc = &(txContext.tx_fields.create_contract);
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //Call contract from
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      //Contract Address
      nuls_address_to_encoded_base58(cc->contractAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 3:
      //Value
      amountTextSize = nuls_hex_amount_to_displayable(cc->value, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 4:
      //Gas Limit
      amountTextSize = nuls_hex_amount_to_displayable(cc->gasLimit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 5:
      //Price
      amountTextSize = nuls_hex_amount_to_displayable(cc->price, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 6:
      //Fees
      amountTextSize = nuls_hex_amount_to_displayable(txContext.fees, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 7:
      //Code Hash
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              8, cc->codeDigest,
              8, cc->codeDigest + DIGEST_LENGTH - 8);
      break;
    case 8:
      //Args
      amountTextSize = MIN(50, cc->argsSize);
      os_memmove(lineBuffer, cc->args, amountTextSize);
      if(amountTextSize < 46) { //Remove tailing ", "
        lineBuffer[amountTextSize - 3] = '\0';
      } else {
        os_memmove(lineBuffer + 46, "...\0", 4);
      }
      break;
    default:
      THROW(INVALID_STATE);
  }
}

void tx_parse_specific_100_create_contract() {

  /* TX Structure:
   *
   * COMMON
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   *
   * TX_SPECIFIC (handled here)
   * - sender - ADDRESS_LENGTH
   * - contractAddress - ADDRESS_LENGTH
   * - value - AMOUNT_LENGTH
   * - codelen - 32bytes
   * - code
   * - gasLimit - AMOUNT_LENGTH
   * - price - AMOUNT_LENGTH
   * - argsn - 1 byte -> number of args (loop)
   *   -argn - 1 byte -> number of arg items
   *      - len:argitem
   *
   * COIN_INPUT (multiple)
   * - owner (hash + index)
   * - amount
   * - locktime
   * COIN_OUTPUT (change)
   * - owner (address only)
   * - amount
   * - locktime
   * */

  tx_type_specific_100_create_contract_t *cc = &(txContext.tx_fields.create_contract);
  uint64_t tmpVarInt;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");
      //init sha256
      cx_sha256_init(&cc->codeHash);

    case _100_CREATE_CONTRACT_SENDER:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_SENDER;
      PRINTF("-- _100_CREATE_CONTRACT_SENDER\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->sender, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("sender: %.*H\n", ADDRESS_LENGTH, cc->sender);

      //Check here that sender is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->sender, ADDRESS_LENGTH) != 0) {
        // PRINTF(("Deposit address is different from account provided in input!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _100_CREATE_CONTRACT_CADDRESS:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CADDRESS;
      PRINTF("-- _100_CREATE_CONTRACT_CADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->contractAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("contractAddress: %.*H\n", ADDRESS_LENGTH, cc->contractAddress);

    case _100_CREATE_CONTRACT_VALUE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_VALUE;
      PRINTF("-- _100_CREATE_CONTRACT_VALUE\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->value, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("value: %.*H\n", AMOUNT_LENGTH, cc->value);

    case _100_CREATE_CONTRACT_CODELEN:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_VALUE;
      PRINTF("-- _100_CREATE_CONTRACT_VALUE\n");
      is_available_to_parse(4);
      cc->codeLen = nuls_read_u32(txContext.bufferPointer, 1, 0);
      cc->codeLenMissing = cc->codeLen;
      transaction_offset_increase(4);
      PRINTF("codeLen: %d\n", cc->codeLen);

    case _100_CREATE_CONTRACT_CODE_LENGTH:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CODE_LENGTH;
      PRINTF("-- _100_CREATE_CONTRACT_CODE_LENGTH\n");
      tmpVarInt = transaction_get_varint();

      if(tmpVarInt != cc->codeLen) {
        PRINTF(("CodeLen is different from VarInt!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _100_CREATE_CONTRACT_CODE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CODE;
      PRINTF("-- _100_CREATE_CONTRACT_CODE\n");
      PRINTF("Missing Data: %d\n", cc->codeLenMissing);
      PRINTF("Current Chunk Size: %d\n", txContext.bytesChunkRemaining);

      tmpVarInt = MIN(cc->codeLenMissing, txContext.bytesChunkRemaining);
      cx_hash(&cc->codeHash, 0, txContext.bufferPointer, tmpVarInt, NULL, 0);
      cc->codeLenMissing -= tmpVarInt;
      transaction_offset_increase(tmpVarInt);

      //Check if we need next chunk
      if(txContext.bytesChunkRemaining == 0 && cc->codeLenMissing != 0) {
        PRINTF("codeLenMissing is not 0 - we need next chunk.\n");
        THROW(NEED_NEXT_CHUNK);
      }

      if(cc->codeLenMissing == 0) {
        PRINTF("codeLenMissing is 0 - let's finalize the data hash\n");
        //let's finalize the hash
        unsigned char fake[1];
        cx_hash(&cc->codeHash.header, CX_LAST, fake, 0, cc->codeDigest, DIGEST_LENGTH);
        PRINTF("Code Digest %.*H\n", DIGEST_LENGTH, cc->codeDigest);
      }

    case _100_CREATE_CONTRACT_GASLIMIT:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_GASLIMIT;
      PRINTF("-- _100_CREATE_CONTRACT_GASLIMIT\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->gasLimit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("gasLimit: %.*H\n", AMOUNT_LENGTH, cc->gasLimit);

    case _100_CREATE_CONTRACT_PRICE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_PRICE;
      PRINTF("-- _100_CREATE_CONTRACT_PRICE\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->price, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("price: %.*H\n", AMOUNT_LENGTH, cc->price);

    case _100_CREATE_CONTRACT_ARGS_I:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_I; // Will be done in the next switch
    case _100_CREATE_CONTRACT_ARGS_J:
    case _100_CREATE_CONTRACT_ARG_LENGTH:
    case _100_CREATE_CONTRACT_ARG:
      break; // Go inside do-while

    default:
      THROW(INVALID_STATE);
  }

  do {

    switch(txContext.tx_parsing_state) {

      case _100_CREATE_CONTRACT_ARGS_I:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_I;
        PRINTF("-- _100_CREATE_CONTRACT_ARGS_I\n");
        is_available_to_parse(1);
        cc->arg_i = txContext.bufferPointer[0];
        cc->curr_i = 0;
        transaction_offset_increase(1);
        PRINTF("-- args i: %d\n", cc->arg_i);

      case _100_CREATE_CONTRACT_ARGS_J:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_J;
        PRINTF("-- _100_CREATE_CONTRACT_ARGS_J\n");
        is_available_to_parse(1);
        cc->arg_j = txContext.bufferPointer[0];
        cc->curr_j = 0;
        transaction_offset_increase(1);
        PRINTF("-- args j: %d\n", cc->arg_j);

      case _100_CREATE_CONTRACT_ARG_LENGTH:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG_LENGTH;
        PRINTF("-- _100_CREATE_CONTRACT_ARG_LENGTH\n");
        cc->argLength = transaction_get_varint();
        PRINTF("-- args [%d][%d] length: %d\n", cc->curr_i, cc->curr_j, (unsigned char) cc->argLength);

      case _100_CREATE_CONTRACT_ARG:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG;
        PRINTF("-- _100_CREATE_CONTRACT_ARG\n");
        is_available_to_parse(cc->argLength);

        char charToVideo = MIN(50 - cc->argsSize - 3, cc->argLength); // -3 because of ", \0" at the end
        PRINTF("-- charToVideo: %d\n", charToVideo);
        PRINTF("-- current argSize: %d\n", cc->argsSize);
        if(charToVideo > 0 && cc->argsSize < ( 50 - charToVideo )) {
          //if cc->argsSize is not 0, remove \0
          if(cc->argsSize > 0) cc->argsSize--;
          os_memmove(cc->args + cc->argsSize, txContext.bufferPointer, charToVideo);
          cc->argsSize += charToVideo;
          cc->args[cc->argsSize] = ',';
          cc->args[cc->argsSize + 1] = ' ';
          cc->args[cc->argsSize + 2] = '\0';
          cc->argsSize += 3;
        }

        transaction_offset_increase(cc->argLength);
        PRINTF("-- temp arg display: %s\n", cc->args);


        //Calculate next state
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG_LENGTH;
        cc->curr_j++;
        if(cc->curr_j == cc->arg_j) {
          txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_J;
          cc->curr_i++;
        }
        PRINTF("-- arg_i: %d\n", cc->arg_i);
        PRINTF("-- curr_i: %d\n", cc->curr_i);
        PRINTF("-- arg_j: %d\n", cc->arg_j);
        PRINTF("-- curr_j: %d\n", cc->curr_j);
        break;

      default:
        THROW(INVALID_STATE);

    }

  }
  while (cc->curr_i < cc->arg_i);

  PRINTF("-- OUT FROM tx_parse_specific_100_create_contract\n");

  //It's time for CoinData
  txContext.tx_parsing_group = COIN_INPUT;
  txContext.tx_parsing_state = BEGINNING;
}

void tx_finalize_100_create_contract() {
  PRINTF("tx_finalize_100_create_contract\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0 || (reqContext.accountChange.pathLength > 0 && !txContext.changeFound)) {
    // PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_100_create_contract - A\n");

  /*
  PRINTF("tx_finalize_100_create_contract - nOut %d\n", txContext.nOut);
  PRINTF("tx_finalize_100_create_contract - txContext.outputAddress %.*H\n", ADDRESS_LENGTH, txContext.outputAddress[0]);
  PRINTF("tx_finalize_100_create_contract - BLACK_HOLE_ADDRESS %.*H\n", ADDRESS_LENGTH, BLACK_HOLE_ADDRESS);
  PRINTF("tx_finalize_100_create_contract - txContext.outputAmount %.*H\n", AMOUNT_LENGTH, txContext.outputAmount[0]);
  PRINTF("tx_finalize_100_create_contract - BLACK_HOLE_ALIAS_AMOUNT %.*H\n", AMOUNT_LENGTH, BLACK_HOLE_ALIAS_AMOUNT);
  PRINTF("tx_finalize_100_create_contract - nuls_secure_memcmp address %d\n", nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH));
  PRINTF("tx_finalize_100_create_contract - nuls_secure_memcmp amount %d\n", nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH));
  */

  PRINTF("tx_finalize_100_create_contract - B\n");

  //Calculate fees (input - output)
  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_100_create_contract - C\n");

  PRINTF("finalize. Fees: %.*H\n", AMOUNT_LENGTH, txContext.fees);

  ux.elements = ui_100_create_contract_nano;
  ux.elements_count = 19;
  totalSteps = 8;
  step_processor = stepProcessor_100_create_contract;
  ui_processor = uiProcessor_100_create_contract;
}
