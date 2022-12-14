import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "L1StandardBridge"
const contractAddr = "0x99c9fc46f92e8a1c0dec1b1747d010903e884be1";
const testNetwork = "ethereum";
const txValue = '0.001';

const testLabel = "optimism bridge"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0xcd65a6d97e92b651c28979de32ff2c2ec39d7bb752982877f14916b04fa846c8
const inputData = "0x"
const models = [
    {
        name: 'nanos',
        steps: 6
    },
    // {
    // 	name: 'nanox',
    // 	steps: 0
    // },
]

// populate unsignedTx from genericTx and get network chain id
const unsignedTx = populateTransaction(contractAddr, inputData, testNetwork, txValue);
// Process tests for each nano models
models.forEach((model) => {
    const nano_model = nano_models.find((nano_model) => nano_model.name === model.name)
    processTest(nano_model, model.steps, contractName, testLabel, testDirSuffix, unsignedTx, testNetwork)
})
