#include "seaport_plugin.h"

// EDIT THIS: Adapt this function to your needs! Remember, the information for
// tokens are held in `msg->item1` and `msg->item2`. If those pointers are
// `NULL`, this means the ethereum app didn't find any info regarding the
// requested tokens!

void handle_provide_token(void *parameters) {
  ethPluginProvideInfo_t *msg = (ethPluginProvideInfo_t *)parameters;
  context_t *context = (context_t *)msg->pluginContext;

  // if (context->order_type != ETH_NFT) {
  if (context->order_type == NFT_ERC20 || context->order_type == ERC20_NFT) {
    if (msg->item1) {
      if (context->order_type == NFT_ERC20) {
        strlcpy(context->token1_ticker, (char *)msg->item1->nft.collectionName,
                sizeof(context->token1_ticker));
      } else if (context->order_type == ERC20_NFT) {
        context->token1_decimals = msg->item1->token.decimals;
        strlcpy(context->token1_ticker, (char *)msg->item1->token.ticker,
                sizeof(context->token1_ticker));
      }
      // no item1
    } else {
      PRINTF("handle_provide_token NO item1\n");
      strlcpy(context->token1_ticker, "NFT", sizeof(context->token1_ticker));
    }
    // is ETH_NFT
  } else {
    strlcpy(context->token1_ticker, ETH, sizeof(context->token1_ticker));
  }

  if (msg->item2) {
    if (context->order_type == NFT_ERC20) {
      context->token2_decimals = msg->item2->token.decimals;
      strlcpy(context->token2_ticker, (char *)msg->item2->token.ticker,
              sizeof(context->token2_ticker));
    } else if (context->order_type == ERC20_NFT || context->order_type == ETH_NFT) {
      strlcpy(context->token2_ticker, (char *)msg->item2->nft.collectionName,
              sizeof(context->token2_ticker));
    }
    // no item2
  } else {
    PRINTF("handle_provide_token NO item2\n");
    if (context->order_type == ETH_NFT || context->order_type == ERC20_NFT)
      strlcpy(context->token2_ticker, "NFT", sizeof(context->token2_ticker));
  }

  msg->result = ETH_PLUGIN_RESULT_OK;
}
