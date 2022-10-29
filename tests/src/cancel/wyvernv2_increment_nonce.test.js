import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "WyvernExchangeWithBulkCancellations";
const contractAddr = "0x7f268357a8c2552623316e2562d90e642bb538e5";
const testNetwork = "ethereum";

const testLabel = "WyvernV2 incrementNonce"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0xda6d6594a1428fc03a71887548ea9bb2dcb3062c833368cfa37a2b702c4aea81
const inputData = "0x627cdcb9";
const models = [
    {
        name: 'nanos',
        steps: 4
    },
    // {
    // 	name: 'nanox',
    // 	steps: 0
    // },
]

// populate unsignedTx from genericTx and get network chain id
const unsignedTx = populateTransaction(contractAddr, inputData, testNetwork);
// Process tests for each nano models
models.forEach((model) => {
    const nano_model = nano_models.find((nano_model) => nano_model.name === model.name)
    processTest(nano_model, model.steps, contractName, testLabel, testDirSuffix, unsignedTx, testNetwork)
})
